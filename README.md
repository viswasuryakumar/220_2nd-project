# SimpleCPU16

A 16-bit CPU emulator and assembler written in C for educational purposes.

## What is This?

SimpleCPU16 is a complete software implementation of a 16-bit CPU that can:
- Assemble assembly language programs into binary code
- Execute binary programs with a virtual CPU
- Show step-by-step execution traces for learning

## Quick Start

### 1. Build the Project

This creates:
- `build/assembler` - Converts assembly code to binary
- `build/emulator` - Runs binary programs


## How to Write and Run Your Own Program

### Step 1: Write Assembly Code

Create a file `myprogram.asm`:

```assembly
.ORG 0x0000

start:
    LDI R0, 42          ; Load 42 into register R0
    ST [0xF801], R0     ; Print the number
    HALT                ; Stop execution
```

### Step 2: Assemble It

```bash
./build/assembler myprogram.asm -o myprogram.bin
```

### Step 3: Run It

```bash
./build/emulator myprogram.bin
```

Output:
```
Resu120
R1: 0x0078 (120)
```

## Example Programs Included

### 1. Recursive Factorial (`programs/factorial.asm`)

**NEW!** A complete demonstration of function calls and recursion on SimpleCPU16. This program computes 5! = 120 using recursive function calls.

```bash
make test_factorial
```

**Features demonstrated:**
- Recursive function calls using `CALL` and `RET` instructions
- Stack management with `PUSH` and `POP`
- Function calling convention (arguments in R0, return value in R0)
- Register preservation (callee-saved registers)
- Base case and recursive case handling
- Stack frame creation and unwinding

**Output:**
```
Computing factorial of 5...
Result: 120
```

**What it shows:**
- **Memory Layout**: Code at 0x0000, data at 0xDFE0, stack at 0xE000
- **Function Calls**: 5 nested recursive calls, each with its own stack frame
- **Stack Growth**: Stack grows from 0xE000 down to 0xDFF1 (15 words for 5 calls)
- **Return Values**: Proper unwinding and result computation (1 → 2 → 6 → 24 → 120)

For a complete execution trace and memory layout analysis, see `docs/FACTORIAL_EXECUTION.md`.

**Reference Implementation:**
A C reference implementation is provided in `factorial_reference.c` showing the equivalent code in a high-level language.

## Available Commands

| Command | Description |
|---------|-------------|
| `make all` | Build emulator and assembler |
| `make clean` | Remove all build files |
| `make test_hello` | Run Hello World program |
| `make test_fibonacci` | Run Fibonacci program |
| `make test_timer` | Run Timer demo with trace |
| `make test_factorial` | Run recursive factorial (5! = 120) |
| `make test_all` | Run all test programs |

## Emulator Options

```bash
./build/emulator <program.bin> [options]
```

**Options:**
- `--trace` - Show detailed execution trace (every instruction)
- `--memdump <file>` - Save memory contents to a file
- `--help` - Show help message

## Assembly Language Basics

### Registers

- `R0` to `R7` - General purpose registers
- `R7` (or `SP`) - Stack pointer

### Common Instructions

```assembly
LDI R0, 100        ; Load immediate: R0 = 100
LD R1, [0x1000]    ; Load from memory: R1 = Memory[0x1000]
ST [0x1000], R2    ; Store to memory: Memory[0x1000] = R2
ADD R0, R1         ; Add: R0 = R0 + R1
SUB R0, R1         ; Subtract: R0 = R0 - R1
MUL R0, R1         ; Multiply: R0 = R0 * R1
CMP R0, R1         ; Compare (sets flags)
BEQ label          ; Branch if equal
JMP label          ; Jump to label
PUSH R0            ; Push R0 onto stack
POP R1             ; Pop from stack into R1
CALL function      ; Call function (saves return address)
RET                ; Return from function
HALT               ; Stop execution
```

### Directives

```assembly
.ORG 0x0000            ; Set starting address
.WORD 0x1234           ; Insert raw 16-bit value
.STRING "text"         ; Insert null-terminated string
```

### Comments

```assembly
; This is a comment
LDI R0, 100    ; This is also a comment
```

## Project Structure

```
SimpleCPU16/
├── README.md                   # This file
├── Makefile                    # Build system
├── factorial_reference.c       # C reference for recursive factorial
├── src/                        # Source code
│   ├── emulator/               # CPU Emulator
│   │   ├── cpu.h               # CPU definitions
│   │   ├── cpu.c               # CPU implementation
│   │   └── main.c              # Emulator entry point
│   └── assembler/              # Assembler
│       ├── assembler.h         # Assembler definitions
│       ├── assembler.c         # Assembler implementation
│       └── main.c              # Assembler entry point
├── programs/                   # Example assembly programs
│   └── factorial.asm           # Recursive factorial (NEW!)
├── docs/                       # Documentation
│   ├── ISA.md                  # Complete instruction reference
│   ├── ARCHITECTURE.md         # CPU architecture details
│   ├── cpu_schematic.txt       # ASCII art CPU diagram
│   └── FACTORIAL_EXECUTION.md  # Detailed recursion walkthrough (NEW!)
└── build/                      # Build output (created by make)
    ├── emulator                # Emulator executable
    ├── assembler               # Assembler executable
    └── *.o, *.bin              # Object and binary files
```

## Features

- **8 Registers**: R0-R7 (R7 is stack pointer)
- **64K Memory**: 65,536 words of 16-bit memory
- **35+ Instructions**: Arithmetic, logic, branches, I/O, and more
- **4 Condition Flags**: Zero (Z), Negative (N), Carry (C), Overflow (V)
- **Function Calls**: `CALL` and `RET` instructions for subroutines and recursion
- **Stack Operations**: `PUSH` and `POP` for stack-based programming
- **Memory-Mapped I/O**: For character, integer, and string output
- **Two-Pass Assembler**: Supports labels and forward references
- **Trace Mode**: See exactly what the CPU is doing

## Memory Map

```
0x0000 - 0xDFFF : Program Memory
0xE000 - 0xF7FF : Stack (grows downward)
0xF800 - 0xFFFF : Memory-Mapped I/O
```

### I/O Addresses

| Address | Device | Purpose |
|---------|--------|---------|
| 0xF800 | CHAR_OUT | Write character |
| 0xF801 | INT_OUT | Write integer |
| 0xF802 | STR_OUT | Write string |
| 0xF810 | TIMER | Read cycle counter |
| 0xF820 | CHAR_IN | Read character |

## Documentation

For detailed information:

- **ISA Reference**: See `docs/ISA.md` for complete instruction set
- **Architecture**: See `docs/ARCHITECTURE.md` for CPU design details
- **Diagrams**: See `docs/cpu_schematic.txt` for visual CPU layout
- **Recursion Tutorial**: See `docs/FACTORIAL_EXECUTION.md` for step-by-step recursion walkthrough

## Sample Program Walkthrough

Here's a simple program that adds two numbers:

```assembly
.ORG 0x0000

start:
    LDI R0, 10          ; Load 10 into R0
    LDI R1, 32          ; Load 32 into R1
    ADD R0, R1          ; R0 = R0 + R1 = 42
    ST [0xF801], R0     ; Print result (42)
    HALT                ; Stop
```

**To run it:**

1. Save as `add.asm`
2. Assemble: `./build/assembler add.asm -o add.bin`
3. Run: `./build/emulator add.bin`
4. Output: `42`

## Troubleshooting

### "make: command not found"

Install build tools:
- **macOS**: `xcode-select --install`
- **Linux**: `sudo apt install build-essential`

### "Cannot open binary file"

Make sure you assembled the program first:
```bash
./build/assembler programs/hello.asm -o build/hello.bin
./build/emulator build/hello.bin
```

### "Permission denied"

Make executables runnable:
```bash
chmod +x build/emulator build/assembler
```

## Clean Build

To rebuild everything from scratch:

```bash
make clean
make all
```

## Technical Details

| Specification | Value |
|--------------|-------|
| Word Size | 16 bits |
| Memory Size | 64K words (128 KB) |
| Registers | 8 general-purpose |
| Instruction Length | 1-2 words |
| Opcodes | 16 primary opcodes |
| Addressing Modes | Immediate, Direct, Indirect, Register |

## What You Can Learn

This project demonstrates:

1. **Computer Architecture**: How CPUs fetch, decode, and execute instructions
2. **Assembly Language**: Low-level programming concepts
3. **Compilers**: How assemblers translate code to machine language
4. **Memory Management**: Stack, heap, and memory-mapped I/O
5. **Function Calls & Recursion**: How subroutine calls work at the hardware level
6. **Stack Frames**: How activation records are created and managed
7. **System Programming**: Building system-level tools in C


## License

Educational project - free to use and modify.

## Author

Created for CMPE 220 - Computer Architecture coursework.

---

**Need Help?** Check the documentation in the `docs/` folder for complete details on the instruction set and architecture.
