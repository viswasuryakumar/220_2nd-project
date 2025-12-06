# SimpleCPU16 Architecture

## Overview

SimpleCPU16 is a 16-bit RISC-style software CPU emulator designed for educational purposes. It implements a complete fetch-decode-execute cycle, memory-mapped I/O, and supports a rich instruction set suitable for demonstrating fundamental computer architecture concepts.

---

## Architecture Type

- **Type**: 16-bit RISC-style load/store architecture
- **Endianness**: Little-endian
- **Word Size**: 16 bits
- **Instruction Length**: Variable (1-2 words)

---

## CPU Components

### 1. Registers

The CPU contains 8 general-purpose 16-bit registers:

| Register | Number | Special Purpose | Description |
|----------|--------|----------------|-------------|
| R0 | 0 | General | General-purpose register |
| R1 | 1 | General | General-purpose register |
| R2 | 2 | General | General-purpose register |
| R3 | 3 | General | General-purpose register |
| R4 | 4 | General | General-purpose register |
| R5 | 5 | General | General-purpose register |
| R6 | 6 | General | General-purpose register |
| R7 (SP) | 7 | Stack Pointer | Stack pointer (initialized to 0xE000) |

#### Special Registers

| Register | Width | Description |
|----------|-------|-------------|
| PC | 16-bit | Program Counter - points to next instruction |
| IR | 16-bit | Instruction Register - holds current instruction |
| FLAGS | 4-bit | Condition flags (Z, N, C, V) |

### 2. Arithmetic Logic Unit (ALU)

The ALU performs the following operations:

**Arithmetic Operations:**
- Addition (ADD, ADDI, INC)
- Subtraction (SUB, SUBI, DEC)
- Multiplication (MUL) - lower 16 bits of result
- Division (DIV) - integer division

**Logical Operations:**
- Bitwise AND
- Bitwise OR
- Bitwise XOR
- Bitwise NOT

**Shift Operations:**
- Logical Shift Left (SHL)
- Logical Shift Right (SHR)
- Arithmetic Shift Right (SAR) - sign-extending

**Comparison:**
- CMP - subtracts operands and sets flags without storing result

### 3. Control Unit

The Control Unit orchestrates the fetch-decode-execute cycle:

1. **FETCH**:
   - Read instruction from memory at address PC
   - Load instruction into IR
   - Increment PC

2. **DECODE**:
   - Extract opcode from bits [15:12]
   - Extract destination register (Rd) from bits [11:9]
   - Extract source register (Rs) from bits [8:6]
   - Extract mode/sub-opcode from bits [5:0]

3. **EXECUTE**:
   - Perform operation based on opcode
   - Read additional words if needed (immediates, addresses)
   - Update registers and/or memory
   - Update flags if applicable

4. **WRITE-BACK**:
   - Results written to destination register or memory
   - Flags updated based on operation

### 4. Memory System

#### Memory Organization

- **Total Address Space**: 65,536 words (64K × 16 bits = 128 KB)
- **Memory Layout**:
  ```
  0x0000 - 0xDFFF : Program Memory (57,344 words)
  0xE000 - 0xF7FF : Stack Area (6,144 words)
  0xF800 - 0xFFFF : Memory-Mapped I/O (2,048 words)
  ```

#### Stack

- **Location**: 0xE000 - 0xF7FF
- **Initial SP**: 0xE000
- **Growth Direction**: Downward (SP decrements on PUSH)
- **Usage**: Function calls, local storage, register preservation

#### Memory-Mapped I/O

The CPU uses memory-mapped I/O for peripheral communication:

| Address | Device | Access | Function |
|---------|--------|--------|----------|
| 0xF800 | CHAR_OUT | Write | Output ASCII character (low byte) |
| 0xF801 | INT_OUT | Write | Output 16-bit decimal integer |
| 0xF802 | STR_OUT | Write | Output null-terminated string |
| 0xF810 | TIMER | Read | Read cycle counter (low 16 bits) |
| 0xF820 | CHAR_IN | Read | Read character from stdin |

### 5. Bus Architecture

The CPU uses a unified memory bus for both instructions and data (Von Neumann architecture):

- **Address Bus**: 16 bits (64K addressable words)
- **Data Bus**: 16 bits
- **Control Signals**: Read/Write enable

---

## Execution Model

### Fetch-Decode-Execute Cycle

```
┌─────────────────────────────────────────┐
│         FETCH PHASE                     │
│  1. IR ← Memory[PC]                     │
│  2. PC ← PC + 1                         │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│         DECODE PHASE                    │
│  1. Extract Opcode [15:12]              │
│  2. Extract Rd [11:9]                   │
│  3. Extract Rs [8:6]                    │
│  4. Extract Mode [5:0]                  │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│         EXECUTE PHASE                   │
│  1. Fetch additional words if needed    │
│  2. Read source operands                │
│  3. Perform ALU operation               │
│  4. Handle branches/jumps               │
└──────────────┬──────────────────────────┘
               │
               ▼
┌─────────────────────────────────────────┐
│         WRITE-BACK PHASE                │
│  1. Write result to Rd or Memory        │
│  2. Update flags (Z, N, C, V)           │
│  3. Increment cycle counter             │
└─────────────────────────────────────────┘
```

### Instruction Pipeline

SimpleCPU16 uses a **single-cycle execution model** for simplicity:
- Each instruction completes in one cycle
- No pipelining or hazard detection
- Memory access completes in the same cycle

---

## Condition Flags

The FLAGS register contains four condition codes:

```
 Bit:  3    2    1    0
      ┌────┬────┬────┬────┐
      │ V  │ C  │ N  │ Z  │
      └────┴────┴────┴────┘
```

| Flag | Bit | Name | Set When |
|------|-----|------|----------|
| Z | 0 | Zero | Result equals zero |
| N | 1 | Negative | Result is negative (bit 15 = 1) |
| C | 2 | Carry | Unsigned overflow/carry out |
| V | 3 | Overflow | Signed overflow (reserved) |

### Flag Usage in Branching

| Branch | Condition | Flags Used |
|--------|-----------|------------|
| BEQ | Equal | Z = 1 |
| BNE | Not Equal | Z = 0 |
| BGT | Greater Than | N = 0 AND Z = 0 |
| BLT | Less Than | N = 1 |
| BGE | Greater or Equal | N = 0 |
| BLE | Less or Equal | N = 1 OR Z = 1 |
| BCS | Carry Set | C = 1 |
| BCC | Carry Clear | C = 0 |

---

## Instruction Encoding

### Instruction Format

All instructions use a base 16-bit format:

```
 15  14  13  12 │ 11  10   9 │  8   7   6 │  5   4   3   2   1   0
┌───┬───┬───┬───┼───┬───┬───┼───┬───┬───┼───┬───┬───┬───┬───┬───┐
│   OPCODE      │     Rd    │     Rs    │      Mode/SubOp       │
└───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
```

- **Opcode [15:12]**: Primary operation (16 opcodes)
- **Rd [11:9]**: Destination register (0-7)
- **Rs [8:6]**: Source register (0-7)
- **Mode [5:0]**: Addressing mode or sub-operation (64 variations)

### Multi-Word Instructions

Instructions requiring immediate values or addresses use two words:

```
Word 1: [Opcode][Rd][Rs][Mode]
Word 2: [Immediate Value or Address]
```

Examples:
- `LDI R0, 1234` → Word1: 0x1200, Word2: 0x04D2
- `JMP 0x0100` → Word1: 0x8000, Word2: 0x0100

---

## Data Path

### Load Operation Data Flow

```
Memory Address Calculation:
    [Rs] or Immediate → Address Bus

Memory Read:
    Memory[Address] → Data Bus → Rd

Example: LD R1, [R2]
    1. Rs (R2) contains address
    2. Memory[R2] is read
    3. Result loaded into Rd (R1)
```

### Arithmetic Operation Data Flow

```
ALU Input:
    Rd → ALU Input A
    Rs → ALU Input B

ALU Operation:
    Result = A op B

Flag Update:
    Z ← (Result == 0)
    N ← (Result[15] == 1)
    C ← Carry/Overflow

Write-Back:
    Result → Rd

Example: ADD R0, R1
    1. R0 → ALU Input A
    2. R1 → ALU Input B
    3. ALU performs A + B
    4. Flags updated
    5. Result → R0
```

### Branch Operation Data Flow

```
Condition Evaluation:
    Flags → Condition Logic

PC Update:
    if (Condition True):
        Target Address → PC
    else:
        PC unchanged

Example: BEQ loop
    1. Check if Z flag = 1
    2. If true: PC ← loop address
    3. If false: continue to next instruction
```

---

## Interrupt and Exception Handling

**Note**: SimpleCPU16 Part 1 does not implement interrupts or exceptions. Features that could be added in future versions:

- Hardware interrupts
- Software interrupts (traps)
- Exception handling (divide by zero, invalid opcode)
- Interrupt vector table
- Interrupt enable/disable flag

---

## Reset and Initialization

On CPU initialization (`cpu_init` or `cpu_reset`):

1. All registers R0-R6 cleared to 0x0000
2. Stack pointer (R7) set to 0xE000
3. Program counter (PC) set to 0x0000
4. Instruction register (IR) cleared
5. All flags (Z, N, C, V) cleared
6. Cycle counter reset to 0
7. Memory initialized to all zeros

---

## Clock and Timing

### Cycle Counter

- 64-bit counter incremented each instruction
- Low 16 bits accessible via MMIO at 0xF810
- Used for timing measurements and benchmarking

### Instruction Timing

All instructions execute in **1 cycle** in this simplified model:
- No wait states
- No cache misses
- Immediate memory access
- No pipeline stalls

---

## Function Call Convention

### CALL Operation

```
CALL function:
    1. SP ← SP - 1          (Allocate stack space)
    2. Memory[SP] ← PC      (Save return address)
    3. PC ← function        (Jump to function)
```

### RET Operation

```
RET:
    1. PC ← Memory[SP]      (Restore return address)
    2. SP ← SP + 1          (Deallocate stack space)
```

### Suggested Calling Convention

While not enforced by hardware, suggested convention:

- **Arguments**: Pass in R0-R3
- **Return value**: Return in R0
- **Preserved registers**: R4-R6 (callee-saved)
- **Temporary registers**: R0-R3 (caller-saved)
- **Stack frame**: Managed by programmer

---

## Design Decisions and Rationale

### 1. Von Neumann Architecture
- **Chosen**: Unified memory for code and data
- **Rationale**: Simplifies design, allows self-modifying code, easier memory management

### 2. 16-bit Word Size
- **Rationale**: Balance between simplicity and capability
- **Trade-offs**: Smaller address space than 32-bit, but easier to understand and implement

### 3. 8 General-Purpose Registers
- **Rationale**: Power of 2 for clean encoding (3 bits), sufficient for most programs
- **Trade-offs**: More than 4 (too few), fewer than 16 (more complex encoding)

### 4. Fixed-Length Primary Instruction
- **Chosen**: All instructions start with 16-bit word
- **Rationale**: Simplifies fetch and decode, predictable timing
- **Extension**: Second word for immediates/addresses when needed

### 5. Memory-Mapped I/O
- **Rationale**: No special I/O instructions needed, unified memory access model
- **Benefits**: Simple to implement, flexible, familiar programming model

### 6. Stack Grows Downward
- **Rationale**: Convention in most architectures, separates stack from program code

### 7. Load/Store Architecture
- **Chosen**: ALU operations only on registers
- **Rationale**: Clean separation of memory access and computation
- **Benefits**: Simpler instruction encoding, faster ALU operations

---

## Comparison with Other Architectures

| Feature | SimpleCPU16 | x86 | ARM | RISC-V |
|---------|-------------|-----|-----|--------|
| Word Size | 16-bit | 32/64-bit | 32/64-bit | 32/64-bit |
| Registers | 8 | 8-16 | 16 | 32 |
| Instruction Length | 1-2 words | Variable | Fixed (32-bit) | Variable |
| Endianness | Little | Little | Bi-endian | Bi-endian |
| I/O Model | Memory-mapped | Port + MMIO | Memory-mapped | Memory-mapped |
| Pipeline | None | Complex | Pipeline | Pipeline |

---

## Performance Characteristics

### Strengths
- Simple, easy to understand
- Predictable execution time
- Low complexity for educational use
- Complete instruction set for basic programs

### Limitations
- No pipelining (lower throughput)
- Limited address space (64K words)
- No cache (slower for large programs)
- No floating-point support
- No SIMD or vector operations
- Single-cycle model (not realistic for modern CPUs)

---

## Future Enhancements (Part 2 and Beyond)

Potential additions for future versions:

1. **Interrupt System**: Hardware and software interrupts
2. **Memory Protection**: User/kernel modes, memory segments
3. **Cache Simulation**: L1/L2 cache with hit/miss statistics
4. **Pipelining**: Multi-stage pipeline with hazard detection
5. **Floating-Point Unit**: IEEE 754 floating-point operations
6. **DMA Controller**: Direct memory access for I/O
7. **MMU**: Virtual memory and address translation
8. **Debugging**: Hardware breakpoints, watchpoints

---

## Verification and Testing

The architecture can be verified through:

1. **Unit Tests**: Individual instruction testing
2. **Integration Tests**: Program-level testing (Hello World, Fibonacci, Timer)
3. **Trace Mode**: Step-by-step execution visualization
4. **Memory Dumps**: Post-execution state inspection
5. **Cycle Counting**: Performance measurement

---

## Implementation Notes

### Emulator Design

The CPU emulator is implemented in C with the following structure:

- **cpu.h**: Type definitions, constants, function prototypes
- **cpu.c**: Core CPU implementation
  - `cpu_init()`: Initialize CPU state
  - `cpu_reset()`: Reset CPU to initial state
  - `cpu_load_program()`: Load program into memory
  - `cpu_run()`: Execute until HALT
  - `cpu_step()`: Execute single instruction
  - `cpu_fetch()`: Fetch instruction from memory
  - `cpu_decode_execute()`: Decode and execute instruction
  - `cpu_read_memory()`: Read from memory (handles MMIO)
  - `cpu_write_memory()`: Write to memory (handles MMIO)
  - `cpu_update_flags()`: Update condition flags
  - `cpu_dump_memory()`: Dump memory to file
  - `cpu_dump_registers()`: Display register state

### Assembler Design

Two-pass assembler implementation:

- **Pass 1**: Collect labels and calculate addresses
- **Pass 2**: Generate machine code with label resolution

---

## Conclusion

SimpleCPU16 provides a complete, functional 16-bit CPU architecture suitable for educational purposes. It demonstrates fundamental concepts of computer architecture including:

- Register-based computation
- Memory hierarchy
- Instruction encoding and decoding
- Fetch-execute cycle
- Addressing modes
- Condition flags and branching
- Stack operations
- Function calls
- Memory-mapped I/O

The architecture is intentionally simplified to focus on core concepts while remaining powerful enough to execute real programs.

---

*End of Architecture Documentation*
