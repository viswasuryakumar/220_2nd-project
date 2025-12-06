#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Initializing CPU state
void cpu_init(CPU* cpu) {
    memset(cpu, 0, sizeof(CPU));
    cpu->registers[REG_SP] = STACK_START;
    cpu->pc = 0;
    cpu->halted = false;
    cpu->cycle_count = 0;
}

// Resetting CPU to initial state
void cpu_reset(CPU* cpu) {
    for (int i = 0; i < NUM_REGISTERS - 1; i++) {
        cpu->registers[i] = 0;
    }
    cpu->registers[REG_SP] = STACK_START;
    cpu->pc = 0;
    cpu->ir = 0;
    cpu->flags.Z = false;
    cpu->flags.N = false;
    cpu->flags.C = false;
    cpu->flags.V = false;
    cpu->halted = false;
    cpu->cycle_count = 0;
}

// Loading program into memory
void cpu_load_program(CPU* cpu, const uint16_t* program, uint16_t size, uint16_t start_addr) {
    if (start_addr + size > MEM_SIZE) {
        fprintf(stderr, "Error: Program too large for memory\n");
        return;
    }
    
    memcpy(&cpu->memory[start_addr], program, size * sizeof(uint16_t));
    cpu->pc = start_addr;
    
    printf("Program loaded: %d words at address 0x%04X\n", size, start_addr);
}

// Reading from memory with MMIO support
uint16_t cpu_read_memory(CPU* cpu, uint16_t address) {
    // Handling memory-mapped I/O reads
    if (address >= MMIO_START) {
        switch (address) {
            case MMIO_TIMER:
                return (uint16_t)(cpu->cycle_count & 0xFFFF);
            case MMIO_CHAR_IN:
                return (uint16_t)getchar();
            default:
                return 0;
        }
    }
    
    return cpu->memory[address];
}

// Writing to memory with MMIO support
void cpu_write_memory(CPU* cpu, uint16_t address, uint16_t value) {
    // Handling memory-mapped I/O writes
    if (address >= MMIO_START) {
        switch (address) {
            case MMIO_CHAR_OUT:
                putchar((char)(value & 0xFF));
                fflush(stdout);
                break;
            case MMIO_INT_OUT:
                printf("%d\n", value);
                fflush(stdout);
                break;
            case MMIO_STR_OUT: {
                // Printing null-terminated string from memory (packed 2 chars per word)
                uint16_t str_addr = value;
                while (1) {
                    uint16_t word = cpu->memory[str_addr++];
                    // Check low byte
                    if ((word & 0xFF) == 0) break;
                    putchar((char)(word & 0xFF));
                    // Check high byte
                    if ((word >> 8) == 0) break;
                    putchar((char)(word >> 8));
                }
                fflush(stdout);
                break;
            }
            default:
                // Ignoring write to other MMIO addresses
                break;
        }
        return;
    }
    
    cpu->memory[address] = value;
}

// Updating CPU flags based on result
void cpu_update_flags(CPU* cpu, uint16_t result, bool update_carry, uint32_t full_result) {
    cpu->flags.Z = (result == 0);
    cpu->flags.N = (result & 0x8000) != 0;
    
    if (update_carry) {
        cpu->flags.C = (full_result > 0xFFFF);
    }
}

// Fetching next instruction
uint16_t cpu_fetch(CPU* cpu) {
    uint16_t instruction = cpu_read_memory(cpu, cpu->pc);
    cpu->ir = instruction;
    cpu->pc++;
    return instruction;
}

// Decoding and executing instruction
void cpu_decode_execute(CPU* cpu, uint16_t instruction, bool trace) {
    // Extracting opcode and operands
    uint8_t opcode = (instruction >> 12) & 0xF;
    uint8_t rd = (instruction >> 9) & 0x7;
    uint8_t rs = (instruction >> 6) & 0x7;
    uint8_t mode = instruction & 0x3F;
    
    if (trace) {
        printf("  [EXECUTE] PC=0x%04X, IR=0x%04X, OP=%X, Rd=R%d, Rs=R%d, Mode=%02X\n",
               cpu->pc - 1, instruction, opcode, rd, rs, mode);
    }
    
    uint16_t operand, addr, result;
    uint32_t full_result;
    int16_t signed_a, signed_result;
    
    switch (opcode) {
        case OP_NOP:
            // No operation
            break;
            
        case OP_LOAD:
            switch (mode) {
                case LOAD_IMM:
                    // Loading immediate value
                    operand = cpu_fetch(cpu);
                    cpu->registers[rd] = operand;
                    if (trace) printf("    LDI R%d, 0x%04X\n", rd, operand);
                    break;
                case LOAD_DIR:
                    // Loading from direct address
                    addr = cpu_fetch(cpu);
                    cpu->registers[rd] = cpu_read_memory(cpu, addr);
                    if (trace) printf("    LD R%d, [0x%04X]\n", rd, addr);
                    break;
                case LOAD_IND:
                    // Loading from indirect address (register contains address)
                    addr = cpu->registers[rs];
                    cpu->registers[rd] = cpu_read_memory(cpu, addr);
                    if (trace) printf("    LD R%d, [R%d] (addr=0x%04X)\n", rd, rs, addr);
                    break;
            }
            break;
            
        case OP_STORE:
            switch (mode) {
                case STORE_DIR:
                    // Storing to direct address
                    addr = cpu_fetch(cpu);
                    cpu_write_memory(cpu, addr, cpu->registers[rs]);
                    if (trace) printf("    ST [0x%04X], R%d\n", addr, rs);
                    break;
                case STORE_IND:
                    // Storing to indirect address
                    addr = cpu->registers[rd];
                    cpu_write_memory(cpu, addr, cpu->registers[rs]);
                    if (trace) printf("    ST [R%d], R%d (addr=0x%04X)\n", rd, rs, addr);
                    break;
            }
            break;
            
        case OP_MOVE:
            // Moving data between registers
            cpu->registers[rd] = cpu->registers[rs];
            if (trace) printf("    MOV R%d, R%d\n", rd, rs);
            break;
            
        case OP_ARITH:
            switch (mode) {
                case ARITH_ADD:
                    full_result = (uint32_t)cpu->registers[rd] + (uint32_t)cpu->registers[rs];
                    result = (uint16_t)full_result;
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, true, full_result);
                    if (trace) printf("    ADD R%d, R%d\n", rd, rs);
                    break;
                case ARITH_SUB:
                    full_result = (uint32_t)cpu->registers[rd] - (uint32_t)cpu->registers[rs];
                    result = (uint16_t)full_result;
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, true, full_result);
                    if (trace) printf("    SUB R%d, R%d\n", rd, rs);
                    break;
                case ARITH_MUL:
                    full_result = (uint32_t)cpu->registers[rd] * (uint32_t)cpu->registers[rs];
                    result = (uint16_t)full_result;
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, true, full_result);
                    if (trace) printf("    MUL R%d, R%d\n", rd, rs);
                    break;
                case ARITH_DIV:
                    if (cpu->registers[rs] != 0) {
                        result = cpu->registers[rd] / cpu->registers[rs];
                        cpu->registers[rd] = result;
                        cpu_update_flags(cpu, result, false, 0);
                    }
                    if (trace) printf("    DIV R%d, R%d\n", rd, rs);
                    break;
                case ARITH_INC:
                    cpu->registers[rd]++;
                    cpu_update_flags(cpu, cpu->registers[rd], false, 0);
                    if (trace) printf("    INC R%d\n", rd);
                    break;
                case ARITH_DEC:
                    cpu->registers[rd]--;
                    cpu_update_flags(cpu, cpu->registers[rd], false, 0);
                    if (trace) printf("    DEC R%d\n", rd);
                    break;
                case ARITH_ADDI:
                    operand = cpu_fetch(cpu);
                    full_result = (uint32_t)cpu->registers[rd] + (uint32_t)operand;
                    result = (uint16_t)full_result;
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, true, full_result);
                    if (trace) printf("    ADDI R%d, 0x%04X\n", rd, operand);
                    break;
                case ARITH_SUBI:
                    operand = cpu_fetch(cpu);
                    full_result = (uint32_t)cpu->registers[rd] - (uint32_t)operand;
                    result = (uint16_t)full_result;
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, true, full_result);
                    if (trace) printf("    SUBI R%d, 0x%04X\n", rd, operand);
                    break;
            }
            break;
            
        case OP_LOGIC:
            switch (mode) {
                case LOGIC_AND:
                    result = cpu->registers[rd] & cpu->registers[rs];
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, false, 0);
                    if (trace) printf("    AND R%d, R%d\n", rd, rs);
                    break;
                case LOGIC_OR:
                    result = cpu->registers[rd] | cpu->registers[rs];
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, false, 0);
                    if (trace) printf("    OR R%d, R%d\n", rd, rs);
                    break;
                case LOGIC_XOR:
                    result = cpu->registers[rd] ^ cpu->registers[rs];
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, false, 0);
                    if (trace) printf("    XOR R%d, R%d\n", rd, rs);
                    break;
                case LOGIC_NOT:
                    result = ~cpu->registers[rd];
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, false, 0);
                    if (trace) printf("    NOT R%d\n", rd);
                    break;
            }
            break;
            
        case OP_SHIFT:
            switch (mode) {
                case SHIFT_LEFT:
                    result = cpu->registers[rd] << (cpu->registers[rs] & 0xF);
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, false, 0);
                    if (trace) printf("    SHL R%d, R%d\n", rd, rs);
                    break;
                case SHIFT_RIGHT:
                    result = cpu->registers[rd] >> (cpu->registers[rs] & 0xF);
                    cpu->registers[rd] = result;
                    cpu_update_flags(cpu, result, false, 0);
                    if (trace) printf("    SHR R%d, R%d\n", rd, rs);
                    break;
                case SHIFT_ARITH:
                    signed_a = (int16_t)cpu->registers[rd];
                    signed_result = signed_a >> (cpu->registers[rs] & 0xF);
                    cpu->registers[rd] = (uint16_t)signed_result;
                    cpu_update_flags(cpu, cpu->registers[rd], false, 0);
                    if (trace) printf("    SAR R%d, R%d\n", rd, rs);
                    break;
            }
            break;
            
        case OP_BRANCH: {
            addr = cpu_fetch(cpu);
            bool should_branch = false;
            
            switch (mode) {
                case BRANCH_EQ:
                    should_branch = cpu->flags.Z;
                    if (trace) printf("    BEQ 0x%04X (Z=%d)\n", addr, cpu->flags.Z);
                    break;
                case BRANCH_NE:
                    should_branch = !cpu->flags.Z;
                    if (trace) printf("    BNE 0x%04X (Z=%d)\n", addr, cpu->flags.Z);
                    break;
                case BRANCH_GT:
                    should_branch = !cpu->flags.N && !cpu->flags.Z;
                    if (trace) printf("    BGT 0x%04X (N=%d,Z=%d)\n", addr, cpu->flags.N, cpu->flags.Z);
                    break;
                case BRANCH_LT:
                    should_branch = cpu->flags.N;
                    if (trace) printf("    BLT 0x%04X (N=%d)\n", addr, cpu->flags.N);
                    break;
                case BRANCH_GE:
                    should_branch = !cpu->flags.N;
                    if (trace) printf("    BGE 0x%04X (N=%d)\n", addr, cpu->flags.N);
                    break;
                case BRANCH_LE:
                    should_branch = cpu->flags.N || cpu->flags.Z;
                    if (trace) printf("    BLE 0x%04X (N=%d,Z=%d)\n", addr, cpu->flags.N, cpu->flags.Z);
                    break;
                case BRANCH_CS:
                    should_branch = cpu->flags.C;
                    if (trace) printf("    BCS 0x%04X (C=%d)\n", addr, cpu->flags.C);
                    break;
                case BRANCH_CC:
                    should_branch = !cpu->flags.C;
                    if (trace) printf("    BCC 0x%04X (C=%d)\n", addr, cpu->flags.C);
                    break;
            }
            
            if (should_branch) {
                cpu->pc = addr;
                if (trace) printf("    -> Branch taken to 0x%04X\n", addr);
            }
            break;
        }
            
        case OP_JUMP:
            addr = cpu_fetch(cpu);
            cpu->pc = addr;
            if (trace) printf("    JMP 0x%04X\n", addr);
            break;
            
        case OP_STACK:
            switch (mode) {
                case STACK_PUSH:
                    cpu->registers[REG_SP]--;
                    cpu_write_memory(cpu, cpu->registers[REG_SP], cpu->registers[rs]);
                    if (trace) printf("    PUSH R%d (SP=0x%04X)\n", rs, cpu->registers[REG_SP]);
                    break;
                case STACK_POP:
                    cpu->registers[rd] = cpu_read_memory(cpu, cpu->registers[REG_SP]);
                    cpu->registers[REG_SP]++;
                    if (trace) printf("    POP R%d (SP=0x%04X)\n", rd, cpu->registers[REG_SP]);
                    break;
            }
            break;
            
        case OP_CALL:
            addr = cpu_fetch(cpu);
            // Pushing return address onto stack
            cpu->registers[REG_SP]--;
            cpu_write_memory(cpu, cpu->registers[REG_SP], cpu->pc);
            cpu->pc = addr;
            if (trace) printf("    CALL 0x%04X (return addr=0x%04X)\n", addr, cpu->pc);
            break;
            
        case OP_RET:
            // Popping return address from stack
            cpu->pc = cpu_read_memory(cpu, cpu->registers[REG_SP]);
            cpu->registers[REG_SP]++;
            if (trace) printf("    RET (return to 0x%04X)\n", cpu->pc);
            break;
            
        case OP_CMP:
            // Comparing two registers (sets flags without storing result)
            full_result = (uint32_t)cpu->registers[rd] - (uint32_t)cpu->registers[rs];
            result = (uint16_t)full_result;
            cpu_update_flags(cpu, result, true, full_result);
            if (trace) printf("    CMP R%d, R%d\n", rd, rs);
            break;
            
        case OP_HALT:
            cpu->halted = true;
            if (trace) printf("    HALT\n");
            break;
            
        default:
            fprintf(stderr, "Unknown opcode: 0x%X at PC=0x%04X\n", opcode, cpu->pc - 1);
            cpu->halted = true;
            break;
    }
}

// Executing single instruction step
void cpu_step(CPU* cpu, bool trace) {
    if (cpu->halted) return;
    
    if (trace) {
        printf("\n[FETCH] PC=0x%04X\n", cpu->pc);
    }
    
    uint16_t instruction = cpu_fetch(cpu);
    cpu_decode_execute(cpu, instruction, trace);
    cpu->cycle_count++;
    
    if (trace) {
        printf("  [WRITE] Registers: ");
        for (int i = 0; i < NUM_REGISTERS; i++) {
            printf("R%d=0x%04X ", i, cpu->registers[i]);
        }
        printf("| Flags: Z=%d N=%d C=%d V=%d\n", 
               cpu->flags.Z, cpu->flags.N, cpu->flags.C, cpu->flags.V);
    }
}

// Running CPU until halt
void cpu_run(CPU* cpu, bool trace) {
    printf("\n=== Starting CPU Execution ===\n");
    
    while (!cpu->halted && cpu->cycle_count < 1000000) {
        cpu_step(cpu, trace);
    }
    
    if (cpu->cycle_count >= 1000000) {
        printf("\n!!! Execution limit reached (possible infinite loop) !!!\n");
    }
    
    printf("\n=== CPU Halted ===\n");
    printf("Total cycles: %llu\n\n", (unsigned long long)cpu->cycle_count);
}

// Dumping registers to console
void cpu_dump_registers(CPU* cpu) {
    printf("\n=== Register Dump ===\n");
    for (int i = 0; i < NUM_REGISTERS; i++) {
        printf("R%d: 0x%04X (%d)\n", i, cpu->registers[i], cpu->registers[i]);
    }
    printf("PC: 0x%04X\n", cpu->pc);
    printf("Flags: Z=%d N=%d C=%d V=%d\n",
           cpu->flags.Z, cpu->flags.N, cpu->flags.C, cpu->flags.V);
    printf("Cycles: %llu\n", (unsigned long long)cpu->cycle_count);
}

// Dumping memory to file
void cpu_dump_memory(CPU* cpu, const char* filename) {
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open file %s for writing\n", filename);
        return;
    }
    
    fprintf(fp, "Memory Dump\n");
    fprintf(fp, "===========\n\n");
    
    for (uint32_t i = 0; i < MEM_SIZE; i++) {
        if (cpu->memory[i] != 0) {
            fprintf(fp, "0x%04X: 0x%04X (%d)\n", i, cpu->memory[i], cpu->memory[i]);
        }
    }
    
    fclose(fp);
    printf("Memory dump written to %s\n", filename);
}
