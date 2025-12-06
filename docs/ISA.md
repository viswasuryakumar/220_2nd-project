# SimpleCPU16 Instruction Set Architecture (ISA)

## Overview

SimpleCPU16 is a 16-bit RISC-style architecture with a custom instruction set. This document provides complete specification of the instruction format, encoding, and behavior.

## Architecture Parameters

- **Word Size**: 16 bits
- **Address Space**: 64K words (65,536 × 16 bits)
- **Registers**: 8 general-purpose registers (R0-R7)
- **Stack Pointer**: R7 (grows downward from 0xE000)
- **Program Counter**: 16-bit PC
- **Instruction Register**: 16-bit IR
- **Flags**: 4 condition flags (Z, N, C, V)

---

## Instruction Format

All instructions follow a 16-bit format with the following bit layout:

```
 15  14  13  12  11  10   9   8   7   6   5   4   3   2   1   0
┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
│   OPCODE  │     Rd    │     Rs    │         Mode/SubOp        │
└───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘
  [15:12]      [11:9]      [8:6]           [5:0]
```

- **OPCODE [15:12]**: 4-bit operation code (16 possible opcodes)
- **Rd [11:9]**: 3-bit destination register (0-7)
- **Rs [8:6]**: 3-bit source register (0-7)
- **Mode/SubOp [5:0]**: 6-bit mode or sub-operation code

### Multi-Word Instructions

Some instructions require additional words for immediate values or addresses:
- Word 1: Instruction encoding
- Word 2: Immediate value or address (when needed)

---

## Processor Flags

The CPU maintains four condition flags that are updated by arithmetic, logical, and comparison operations:

| Flag | Name | Bit | Description |
|------|------|-----|-------------|
| Z | Zero | 0 | Set when result equals zero |
| N | Negative | 1 | Set when result is negative (bit 15 = 1) |
| C | Carry | 2 | Set on arithmetic overflow/carry out |
| V | Overflow | 3 | Set on signed arithmetic overflow |

### Flag Update Rules

| Operation | Z | N | C | V |
|-----------|---|---|---|---|
| ADD/SUB | ✓ | ✓ | ✓ | - |
| MUL/DIV | ✓ | ✓ | ✓ | - |
| INC/DEC | ✓ | ✓ | - | - |
| ADDI/SUBI | ✓ | ✓ | ✓ | - |
| AND/OR/XOR/NOT | ✓ | ✓ | - | - |
| SHL/SHR/SAR | ✓ | ✓ | - | - |
| CMP | ✓ | ✓ | ✓ | - |
| LOAD/STORE/MOV | - | - | - | - |

---

## Opcode Table

| Opcode | Hex | Mnemonic | Description |
|--------|-----|----------|-------------|
| 0x0 | 0x0 | NOP | No operation |
| 0x1 | 0x1 | LOAD | Load operations |
| 0x2 | 0x2 | STORE | Store operations |
| 0x3 | 0x3 | MOVE | Move data between registers |
| 0x4 | 0x4 | ARITH | Arithmetic operations |
| 0x5 | 0x5 | LOGIC | Logical operations |
| 0x6 | 0x6 | SHIFT | Shift operations |
| 0x7 | 0x7 | BRANCH | Conditional branches |
| 0x8 | 0x8 | JUMP | Unconditional jump |
| 0x9 | 0x9 | STACK | Stack operations |
| 0xA | 0xA | CALL | Function call |
| 0xB | 0xB | RET | Return from function |
| 0xC | 0xC | CMP | Compare registers |
| 0xD | 0xD | IO | I/O operations (reserved) |
| 0xE | 0xE | SPEC | Special operations (reserved) |
| 0xF | 0xF | HALT | Halt execution |

---

## Detailed Instruction Specifications

### 1. NOP - No Operation
**Encoding**: `0x0000`
```
Opcode: 0x0, Rd: 0, Rs: 0, Mode: 0x00
```
**Operation**: None
**Flags**: None modified
**Cycles**: 1
**Example**: `NOP`

---

### 2. LOAD Instructions (Opcode 0x1)

#### LDI Rd, imm - Load Immediate
**Encoding**: `0x1RRR000` + immediate word
```
Opcode: 0x1, Rd: RRR, Rs: 000, Mode: 0x00
```
**Operation**: `Rd ← immediate`
**Words**: 2
**Flags**: None
**Example**: `LDI R0, 0x1234` → `R0 = 0x1234`

#### LD Rd, [addr] - Load Direct
**Encoding**: `0x1RRR001` + address word
```
Opcode: 0x1, Rd: RRR, Rs: 000, Mode: 0x01
```
**Operation**: `Rd ← Memory[addr]`
**Words**: 2
**Flags**: None
**Example**: `LD R1, [0x2000]` → `R1 = Memory[0x2000]`

#### LD Rd, [Rs] - Load Indirect
**Encoding**: `0x1RRRSSS02`
```
Opcode: 0x1, Rd: RRR, Rs: SSS, Mode: 0x02
```
**Operation**: `Rd ← Memory[Rs]`
**Words**: 1
**Flags**: None
**Example**: `LD R2, [R3]` → `R2 = Memory[R3]`

---

### 3. STORE Instructions (Opcode 0x2)

#### ST [addr], Rs - Store Direct
**Encoding**: `0x20SSS00` + address word
```
Opcode: 0x2, Rd: 000, Rs: SSS, Mode: 0x00
```
**Operation**: `Memory[addr] ← Rs`
**Words**: 2
**Flags**: None
**Example**: `ST [0x3000], R4` → `Memory[0x3000] = R4`

#### ST [Rd], Rs - Store Indirect
**Encoding**: `0x2RRRSSS01`
```
Opcode: 0x2, Rd: RRR, Rs: SSS, Mode: 0x01
```
**Operation**: `Memory[Rd] ← Rs`
**Words**: 1
**Flags**: None
**Example**: `ST [R5], R6` → `Memory[R5] = R6`

---

### 4. MOVE - Move Register to Register (Opcode 0x3)

**Encoding**: `0x3RRRSSS00`
```
Opcode: 0x3, Rd: RRR, Rs: SSS, Mode: 0x00
```
**Operation**: `Rd ← Rs`
**Words**: 1
**Flags**: None
**Example**: `MOV R0, R1` → `R0 = R1`

---

### 5. Arithmetic Instructions (Opcode 0x4)

#### ADD Rd, Rs - Add Registers
**Encoding**: `0x4RRRSSS00`
```
Mode: 0x00 (ARITH_ADD)
```
**Operation**: `Rd ← Rd + Rs`
**Flags**: Z, N, C
**Example**: `ADD R0, R1` → `R0 = R0 + R1`

#### SUB Rd, Rs - Subtract Registers
**Encoding**: `0x4RRRSSS01`
```
Mode: 0x01 (ARITH_SUB)
```
**Operation**: `Rd ← Rd - Rs`
**Flags**: Z, N, C
**Example**: `SUB R2, R3` → `R2 = R2 - R3`

#### MUL Rd, Rs - Multiply Registers
**Encoding**: `0x4RRRSSS02`
```
Mode: 0x02 (ARITH_MUL)
```
**Operation**: `Rd ← (Rd × Rs) & 0xFFFF` (lower 16 bits)
**Flags**: Z, N, C
**Example**: `MUL R4, R5` → `R4 = (R4 × R5) & 0xFFFF`

#### DIV Rd, Rs - Divide Registers
**Encoding**: `0x4RRRSSS03`
```
Mode: 0x03 (ARITH_DIV)
```
**Operation**: `Rd ← Rd / Rs` (integer division, Rs must be ≠ 0)
**Flags**: Z, N
**Example**: `DIV R6, R7` → `R6 = R6 / R7`

#### INC Rd - Increment Register
**Encoding**: `0x4RRR00004`
```
Mode: 0x04 (ARITH_INC)
```
**Operation**: `Rd ← Rd + 1`
**Flags**: Z, N
**Example**: `INC R0` → `R0 = R0 + 1`

#### DEC Rd - Decrement Register
**Encoding**: `0x4RRR00005`
```
Mode: 0x05 (ARITH_DEC)
```
**Operation**: `Rd ← Rd - 1`
**Flags**: Z, N
**Example**: `DEC R1` → `R1 = R1 - 1`

#### ADDI Rd, imm - Add Immediate
**Encoding**: `0x4RRR00006` + immediate word
```
Mode: 0x06 (ARITH_ADDI)
```
**Operation**: `Rd ← Rd + imm`
**Words**: 2
**Flags**: Z, N, C
**Example**: `ADDI R2, 100` → `R2 = R2 + 100`

#### SUBI Rd, imm - Subtract Immediate
**Encoding**: `0x4RRR00007` + immediate word
```
Mode: 0x07 (ARITH_SUBI)
```
**Operation**: `Rd ← Rd - imm`
**Words**: 2
**Flags**: Z, N, C
**Example**: `SUBI R3, 50` → `R3 = R3 - 50`

---

### 6. Logical Instructions (Opcode 0x5)

#### AND Rd, Rs - Bitwise AND
**Encoding**: `0x5RRRSSS00`
```
Mode: 0x00 (LOGIC_AND)
```
**Operation**: `Rd ← Rd & Rs`
**Flags**: Z, N
**Example**: `AND R0, R1` → `R0 = R0 & R1`

#### OR Rd, Rs - Bitwise OR
**Encoding**: `0x5RRRSSS01`
```
Mode: 0x01 (LOGIC_OR)
```
**Operation**: `Rd ← Rd | Rs`
**Flags**: Z, N
**Example**: `OR R2, R3` → `R2 = R2 | R3`

#### XOR Rd, Rs - Bitwise XOR
**Encoding**: `0x5RRRSSS02`
```
Mode: 0x02 (LOGIC_XOR)
```
**Operation**: `Rd ← Rd ^ Rs`
**Flags**: Z, N
**Example**: `XOR R4, R5` → `R4 = R4 ^ R5`

#### NOT Rd - Bitwise NOT
**Encoding**: `0x5RRR00003`
```
Mode: 0x03 (LOGIC_NOT)
```
**Operation**: `Rd ← ~Rd`
**Flags**: Z, N
**Example**: `NOT R6` → `R6 = ~R6`

---

### 7. Shift Instructions (Opcode 0x6)

#### SHL Rd, Rs - Shift Left Logical
**Encoding**: `0x6RRRSSS00`
```
Mode: 0x00 (SHIFT_LEFT)
```
**Operation**: `Rd ← Rd << (Rs & 0xF)`
**Flags**: Z, N
**Example**: `SHL R0, R1` → `R0 = R0 << (R1 & 0xF)`

#### SHR Rd, Rs - Shift Right Logical
**Encoding**: `0x6RRRSSS01`
```
Mode: 0x01 (SHIFT_RIGHT)
```
**Operation**: `Rd ← Rd >> (Rs & 0xF)` (zero-fill)
**Flags**: Z, N
**Example**: `SHR R2, R3` → `R2 = R2 >> (R3 & 0xF)`

#### SAR Rd, Rs - Shift Right Arithmetic
**Encoding**: `0x6RRRSSS02`
```
Mode: 0x02 (SHIFT_ARITH)
```
**Operation**: `Rd ← Rd >> (Rs & 0xF)` (sign-extend)
**Flags**: Z, N
**Example**: `SAR R4, R5` → `R4 = R4 >> (R5 & 0xF)` (arithmetic)

---

### 8. Branch Instructions (Opcode 0x7)

All branch instructions are 2 words: instruction + target address.

#### BEQ addr - Branch if Equal
**Encoding**: `0x700000000` + address word
```
Mode: 0x00 (BRANCH_EQ)
```
**Condition**: Z = 1
**Operation**: `if (Z) PC ← addr`
**Example**: `BEQ loop` → Branch to 'loop' if zero flag set

#### BNE addr - Branch if Not Equal
**Encoding**: `0x700000001` + address word
```
Mode: 0x01 (BRANCH_NE)
```
**Condition**: Z = 0
**Operation**: `if (!Z) PC ← addr`
**Example**: `BNE loop` → Branch to 'loop' if zero flag clear

#### BGT addr - Branch if Greater Than
**Encoding**: `0x700000002` + address word
```
Mode: 0x02 (BRANCH_GT)
```
**Condition**: N = 0 AND Z = 0
**Operation**: `if (!N && !Z) PC ← addr`
**Example**: `BGT loop` → Branch if result was positive and non-zero

#### BLT addr - Branch if Less Than
**Encoding**: `0x700000003` + address word
```
Mode: 0x03 (BRANCH_LT)
```
**Condition**: N = 1
**Operation**: `if (N) PC ← addr`
**Example**: `BLT loop` → Branch if result was negative

#### BGE addr - Branch if Greater or Equal
**Encoding**: `0x700000004` + address word
```
Mode: 0x04 (BRANCH_GE)
```
**Condition**: N = 0
**Operation**: `if (!N) PC ← addr`
**Example**: `BGE loop` → Branch if result was non-negative

#### BLE addr - Branch if Less or Equal
**Encoding**: `0x700000005` + address word
```
Mode: 0x05 (BRANCH_LE)
```
**Condition**: N = 1 OR Z = 1
**Operation**: `if (N || Z) PC ← addr`
**Example**: `BLE loop` → Branch if result was negative or zero

#### BCS addr - Branch if Carry Set
**Encoding**: `0x700000006` + address word
```
Mode: 0x06 (BRANCH_CS)
```
**Condition**: C = 1
**Operation**: `if (C) PC ← addr`
**Example**: `BCS overflow` → Branch if carry flag set

#### BCC addr - Branch if Carry Clear
**Encoding**: `0x700000007` + address word
```
Mode: 0x07 (BRANCH_CC)
```
**Condition**: C = 0
**Operation**: `if (!C) PC ← addr`
**Example**: `BCC no_overflow` → Branch if carry flag clear

---

### 9. JUMP - Unconditional Jump (Opcode 0x8)

**Encoding**: `0x800000000` + address word
```
Opcode: 0x8, Rd: 000, Rs: 000, Mode: 0x00
```
**Operation**: `PC ← addr`
**Words**: 2
**Example**: `JMP start` → Jump to 'start' label

---

### 10. Stack Instructions (Opcode 0x9)

#### PUSH Rs - Push Register to Stack
**Encoding**: `0x9000SSS00`
```
Mode: 0x00 (STACK_PUSH)
```
**Operation**:
```
SP ← SP - 1
Memory[SP] ← Rs
```
**Example**: `PUSH R0` → Push R0 onto stack

#### POP Rd - Pop from Stack to Register
**Encoding**: `0x9RRR00001`
```
Mode: 0x01 (STACK_POP)
```
**Operation**:
```
Rd ← Memory[SP]
SP ← SP + 1
```
**Example**: `POP R1` → Pop from stack into R1

---

### 11. CALL - Function Call (Opcode 0xA)

**Encoding**: `0xA00000000` + address word
```
Opcode: 0xA, Rd: 000, Rs: 000, Mode: 0x00
```
**Operation**:
```
SP ← SP - 1
Memory[SP] ← PC
PC ← addr
```
**Words**: 2
**Example**: `CALL function` → Call function, save return address

---

### 12. RET - Return from Function (Opcode 0xB)

**Encoding**: `0xB00000000`
```
Opcode: 0xB, Rd: 000, Rs: 000, Mode: 0x00
```
**Operation**:
```
PC ← Memory[SP]
SP ← SP + 1
```
**Words**: 1
**Example**: `RET` → Return from function

---

### 13. CMP - Compare Registers (Opcode 0xC)

**Encoding**: `0xCRRRSSS00`
```
Opcode: 0xC, Rd: RRR, Rs: SSS, Mode: 0x00
```
**Operation**: `temp ← Rd - Rs` (result not stored, only flags updated)
**Flags**: Z, N, C
**Example**: `CMP R0, R1` → Compare R0 with R1, set flags

---

### 14. HALT - Halt Execution (Opcode 0xF)

**Encoding**: `0xF00000000`
```
Opcode: 0xF, Rd: 000, Rs: 000, Mode: 0x00
```
**Operation**: Stop CPU execution
**Example**: `HALT` → Stop program

---

## Memory Map

```
0x0000 - 0xDFFF : Program Memory (57,344 words)
0xE000 - 0xF7FF : Stack Area (6,144 words, grows downward from 0xE000)
0xF800 - 0xFFFF : Memory-Mapped I/O (2,048 words)
```

### Memory Layout Diagram

```
0xFFFF  ┌─────────────────────┐
        │                     │
        │   MMIO Region       │
        │   (I/O Devices)     │
0xF800  ├─────────────────────┤
        │                     │
        │   Stack Area        │
        │   (grows ↓)         │
        │        ↓            │
0xE000  ├─────────────────────┤
        │                     │
        │   Program Memory    │
        │   (Code + Data)     │
        │                     │
0x0000  └─────────────────────┘
```

---

## Memory-Mapped I/O Devices

### Character Output (0xF800)
**Address**: `0xF800` (MMIO_CHAR_OUT)
**Access**: Write-only
**Operation**: Write low 8 bits as ASCII character to stdout
**Example**:
```assembly
LDI R0, 65          ; ASCII 'A'
ST [0xF800], R0     ; Print 'A'
```

### Integer Output (0xF801)
**Address**: `0xF801` (MMIO_INT_OUT)
**Access**: Write-only
**Operation**: Write 16-bit value as decimal integer to stdout
**Example**:
```assembly
LDI R0, 1234
ST [0xF801], R0     ; Print "1234"
```

### String Output (0xF802)
**Address**: `0xF802` (MMIO_STR_OUT)
**Access**: Write-only
**Operation**: Write address of null-terminated string; string is printed
**Format**: One character per word (low byte), terminated by 0x0000
**Example**:
```assembly
LDI R0, message
ST [0xF802], R0     ; Print string at 'message'
```

### Timer Input (0xF810)
**Address**: `0xF810` (MMIO_TIMER)
**Access**: Read-only
**Operation**: Read low 16 bits of cycle counter
**Example**:
```assembly
LD R0, [0xF810]     ; R0 = cycle counter & 0xFFFF
```

### Character Input (0xF820)
**Address**: `0xF820` (MMIO_CHAR_IN)
**Access**: Read-only
**Operation**: Read one character from stdin (blocking)
**Example**:
```assembly
LD R0, [0xF820]     ; R0 = getchar()
```

---

## Addressing Modes

### 1. Immediate Addressing
Value is provided directly in the instruction.
```assembly
LDI R0, 100         ; R0 = 100
```

### 2. Register Addressing
Operands are in registers.
```assembly
ADD R0, R1          ; R0 = R0 + R1
```

### 3. Direct Addressing
Memory address is specified directly.
```assembly
LD R0, [0x2000]     ; R0 = Memory[0x2000]
ST [0x3000], R1     ; Memory[0x3000] = R1
```

### 4. Indirect Addressing
Register contains the memory address.
```assembly
LD R0, [R1]         ; R0 = Memory[R1]
ST [R2], R3         ; Memory[R2] = R3
```

---

## Instruction Timing

All instructions execute in **1 cycle**, except:
- Memory operations complete in the same cycle (simplified model)
- Multi-word instructions fetch additional words (included in cycle count)

---

## Assembly Language Syntax

### Instruction Format
```
[LABEL:]  MNEMONIC  OPERAND1, OPERAND2  ; comment
```

### Register Names
- `R0, R1, R2, R3, R4, R5, R6, R7` (or lowercase `r0-r7`)
- `SP` is an alias for `R7`

### Literals
- **Decimal**: `123`, `-5`
- **Hexadecimal**: `0x1A2B`, `0xFFFF`
- **Character**: `'A'`, `'0'` (converted to ASCII value)

### Directives

#### .ORG address
Set code origin address.
```assembly
.ORG 0x1000
```

#### .WORD value1, value2, ...
Emit raw 16-bit words.
```assembly
.WORD 0x1234, 0x5678, 42
```

#### .STRING "text" or .ASCIIZ "text"
Emit null-terminated string (one char per word).
```assembly
.STRING "Hello"
```

### Comments
Lines or portions starting with `;` are ignored.
```assembly
; This is a full-line comment
LDI R0, 100    ; This is an inline comment
```

---

## Example Programs

### Hello World
```assembly
.ORG 0x0000

start:
    LDI R0, msg
    ST [0xF802], R0     ; Print string
    HALT

msg:
    .STRING "Hello, World!"
```

### Loop Example
```assembly
.ORG 0x0000

    LDI R0, 0           ; Counter
    LDI R1, 10          ; Limit

loop:
    ST [0xF801], R0     ; Print counter
    INC R0
    CMP R0, R1
    BLT loop            ; Continue if R0 < R1

    HALT
```

---

## Instruction Summary Table

| Category | Instructions | Count |
|----------|-------------|-------|
| Data Transfer | LDI, LD, ST, MOV | 4 |
| Arithmetic | ADD, SUB, MUL, DIV, INC, DEC, ADDI, SUBI | 8 |
| Logical | AND, OR, XOR, NOT | 4 |
| Shift | SHL, SHR, SAR | 3 |
| Comparison | CMP | 1 |
| Branch | BEQ, BNE, BGT, BLT, BGE, BLE, BCS, BCC | 8 |
| Jump | JMP | 1 |
| Stack | PUSH, POP | 2 |
| Subroutine | CALL, RET | 2 |
| System | NOP, HALT | 2 |
| **Total** | | **35** |

---

## Notes

1. **Endianness**: Little-endian (least significant byte first)
2. **Stack**: Grows downward (SP decrements on PUSH)
3. **Division by Zero**: Result is undefined; register unchanged
4. **String Format**: One character per 16-bit word (low byte used)
5. **Signed Operations**: Two's complement representation
6. **Overflow Detection**: Carry flag (C) indicates unsigned overflow; overflow flag (V) reserved for signed overflow

---

*End of ISA Specification*
