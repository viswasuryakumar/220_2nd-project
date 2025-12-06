#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdbool.h>

// CPU Architecture Specifications
#define WORD_SIZE 16
#define MEM_SIZE 65536          // 64K words (128KB total)
#define NUM_REGISTERS 8
#define STACK_START 0xE000      // Stack starts at 0xE000
#define MMIO_START 0xF800       // Memory-mapped I/O region

// Register definitions
#define REG_R0 0
#define REG_R1 1
#define REG_R2 2
#define REG_R3 3
#define REG_R4 4
#define REG_R5 5
#define REG_R6 6
#define REG_SP 7                // R7 is Stack Pointer

// Instruction Format:
// [15:12] - Opcode (4 bits, supports 16 base opcodes)
// [11:9]  - Destination Register (3 bits)
// [8:6]   - Source Register (3 bits)
// [5:0]   - Mode/Extended opcode (6 bits)

// Instruction Opcodes
typedef enum {
    OP_NOP    = 0x0,    // No operation
    OP_LOAD   = 0x1,    // Load operations
    OP_STORE  = 0x2,    // Store operations
    OP_MOVE   = 0x3,    // Move data between registers
    OP_ARITH  = 0x4,    // Arithmetic operations
    OP_LOGIC  = 0x5,    // Logical operations
    OP_SHIFT  = 0x6,    // Shift operations
    OP_BRANCH = 0x7,    // Conditional branches
    OP_JUMP   = 0x8,    // Unconditional jumps
    OP_STACK  = 0x9,    // Stack operations
    OP_CALL   = 0xA,    // Function call
    OP_RET    = 0xB,    // Return from function
    OP_CMP    = 0xC,    // Compare operations
    OP_IO     = 0xD,    // I/O operations
    OP_SPEC   = 0xE,    // Special operations
    OP_HALT   = 0xF     // Halt execution
} Opcode;

// Sub-opcodes for ARITH operations
#define ARITH_ADD  0x00
#define ARITH_SUB  0x01
#define ARITH_MUL  0x02
#define ARITH_DIV  0x03
#define ARITH_INC  0x04
#define ARITH_DEC  0x05
#define ARITH_ADDI 0x06    // Add immediate
#define ARITH_SUBI 0x07    // Sub immediate

// Sub-opcodes for LOGIC operations
#define LOGIC_AND  0x00
#define LOGIC_OR   0x01
#define LOGIC_XOR  0x02
#define LOGIC_NOT  0x03

// Sub-opcodes for SHIFT operations
#define SHIFT_LEFT  0x00
#define SHIFT_RIGHT 0x01
#define SHIFT_ARITH 0x02   // Arithmetic right shift

// Sub-opcodes for LOAD operations
#define LOAD_IMM   0x00    // Load immediate (requires extra word)
#define LOAD_DIR   0x01    // Load direct (requires extra word for address)
#define LOAD_IND   0x02    // Load indirect (address in register)

// Sub-opcodes for STORE operations
#define STORE_DIR  0x00    // Store direct (requires extra word for address)
#define STORE_IND  0x01    // Store indirect (address in register)

// Sub-opcodes for BRANCH operations
#define BRANCH_EQ  0x00    // Branch if equal (Z=1)
#define BRANCH_NE  0x01    // Branch if not equal (Z=0)
#define BRANCH_GT  0x02    // Branch if greater (N=0 and Z=0)
#define BRANCH_LT  0x03    // Branch if less than (N=1)
#define BRANCH_GE  0x04    // Branch if greater or equal (N=0)
#define BRANCH_LE  0x05    // Branch if less or equal (N=1 or Z=1)
#define BRANCH_CS  0x06    // Branch if carry set
#define BRANCH_CC  0x07    // Branch if carry clear

// Sub-opcodes for STACK operations
#define STACK_PUSH 0x00
#define STACK_POP  0x01

// Processor Flags
typedef struct {
    bool Z;  // Zero flag
    bool N;  // Negative flag
    bool C;  // Carry flag
    bool V;  // Overflow flag
} Flags;

// CPU State
typedef struct {
    uint16_t registers[NUM_REGISTERS];
    uint16_t pc;                    // Program Counter
    uint16_t ir;                    // Instruction Register
    Flags flags;
    uint16_t memory[MEM_SIZE];
    bool halted;
    uint64_t cycle_count;
} CPU;

// Memory-Mapped I/O Addresses
#define MMIO_CHAR_OUT    0xF800    // Write: Output character (low 8 bits)
#define MMIO_INT_OUT     0xF801    // Write: Output integer (decimal)
#define MMIO_STR_OUT     0xF802    // Write: Output string at address
#define MMIO_TIMER       0xF810    // Read: Cycle counter (low 16 bits)
#define MMIO_CHAR_IN     0xF820    // Read: Input character (blocking)

// Function prototypes
void cpu_init(CPU* cpu);
void cpu_reset(CPU* cpu);
void cpu_load_program(CPU* cpu, const uint16_t* program, uint16_t size, uint16_t start_addr);
void cpu_run(CPU* cpu, bool trace);
void cpu_step(CPU* cpu, bool trace);
void cpu_dump_memory(CPU* cpu, const char* filename);
void cpu_dump_registers(CPU* cpu);

// Helper functions
uint16_t cpu_fetch(CPU* cpu);
void cpu_decode_execute(CPU* cpu, uint16_t instruction, bool trace);
void cpu_update_flags(CPU* cpu, uint16_t result, bool update_carry, uint32_t full_result);
uint16_t cpu_read_memory(CPU* cpu, uint16_t address);
void cpu_write_memory(CPU* cpu, uint16_t address, uint16_t value);

#endif // CPU_H
