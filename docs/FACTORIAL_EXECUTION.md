# Recursive Factorial Execution on SimpleCPU16

## Overview

This document demonstrates how a recursive factorial function is executed on SimpleCPU16, showing memory layout, stack operations, and the complete execution trace.

---

## Program Source

### C Reference Implementation
```c
int factorial(int n) {
    if (n <= 1) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

int main() {
    int result = factorial(5);
    printf("Result: %d\n", result);
    return 0;
}
```

### SimpleCPU16 Assembly Implementation
See `programs/factorial.asm` for the complete implementation.

---

## Memory Layout

### Overall Memory Map

```
┌────────────────────────────────────┐ 0xFFFF
│                                    │
│   Memory-Mapped I/O                │
│   (Character/String Output)        │
│                                    │
├────────────────────────────────────┤ 0xF800
│                                    │
│   Stack Area                       │
│   (grows downward ↓)               │
│   Initial SP: 0xE000               │
│                                    │
├────────────────────────────────────┤ 0xE000
│                                    │
│   Heap / Unused                    │
│                                    │
├────────────────────────────────────┤ 0xDFFF
│   Data Section                     │
│   - msg_computing                  │
│   - msg_result                     │
├────────────────────────────────────┤ 0xDFE0
│                                    │
│   Unused Program Memory            │
│                                    │
├────────────────────────────────────┤ 0x0050
│   Code Section                     │
│   - main                           │
│   - factorial                      │
├────────────────────────────────────┤ 0x0000
```

### Detailed Code Section Layout

```
Address   Content                    Description
─────────────────────────────────────────────────────
0x0000    main:                     Entry point
0x0001      LDI R0, msg_computing   Load message address
0x0003      ST [0xF802], R0         Print message
0x0005      LDI R0, 5               Load argument n=5
0x0007      CALL factorial          Call factorial(5)
0x0009      MOV R1, R0              Save result
0x000A      LDI R0, msg_result      Load result message
0x000C      ST [0xF802], R0         Print message
0x000E      MOV R0, R1              Load result
0x000F      ST [0xF801], R0         Print integer
0x0011      LDI R0, 10              Newline character
0x0013      ST [0xF800], R0         Print character
0x0015      HALT                    Stop execution

0x0016    factorial:                Function entry
0x0017      PUSH R1                 Save R1
0x0018      PUSH R2                 Save R2
0x0019      LDI R1, 1               Load 1 for comparison
0x001B      CMP R0, R1              Check if n <= 1
0x001C      BLE base_case           Branch if base case
0x001E      MOV R2, R0              Save n in R2
0x001F      DEC R0                  n = n - 1
0x0020      CALL factorial          Recursive call
0x0022      MUL R0, R2              R0 = n * factorial(n-1)
0x0023      JMP factorial_return    Skip base case

0x0025    base_case:
0x0025      LDI R0, 1               Return 1

0x0026    factorial_return:
0x0027      POP R2                  Restore R2
0x0028      POP R1                  Restore R1
0x0029      RET                     Return to caller

0xDFE0    msg_computing:            "Computing factorial of 5..."
0xDFF5    msg_result:               "Result: "
```

---

## Stack Frame Structure

Each function call creates a stack frame with this structure:

```
Higher addresses
    ↑
    │
    ├──────────────────┐
    │  Return Address  │ ← Pushed by CALL instruction
    ├──────────────────┤
    │  Saved R1        │ ← Pushed by function prologue
    ├──────────────────┤
    │  Saved R2        │ ← Pushed by function prologue
    ├──────────────────┤ ← SP points here
    │
    ↓
Lower addresses
```

---

## Execution Trace

### Initial State
```
Registers:
  R0-R6: 0x0000
  R7 (SP): 0xE000
  PC: 0x0000
  Flags: Z=0, N=0, C=0

Stack:
  Empty (SP = 0xE000)
```

---

### Step-by-Step Execution

#### **Phase 1: main() prepares to call factorial(5)**

```
PC=0x0000: LDI R0, msg_computing
  → R0 = 0xDFE0

PC=0x0002: ST [0xF802], R0
  → Print "Computing factorial of 5..."

PC=0x0004: LDI R0, 5
  → R0 = 5

PC=0x0006: CALL factorial
  → Stack: [0x0008]              (return address)
  → SP = 0xDFFF
  → PC = 0x0016 (factorial entry)
```

**Stack State After CALL:**
```
Address    Value        Description
─────────────────────────────────
0xE000     [empty]
0xDFFF     0x0008       Return address to main
           ↑ SP
```

---

#### **Phase 2: factorial(5) - First Call**

```
Registers:
  R0 = 5 (argument)
  SP = 0xDFFF
  PC = 0x0016

PC=0x0016: PUSH R1
  → Stack: [0x0008][0x0000]      (ret_addr, saved R1)
  → SP = 0xDFFE

PC=0x0017: PUSH R2
  → Stack: [0x0008][0x0000][0x0000]
  → SP = 0xDFFD

PC=0x0018: LDI R1, 1
  → R1 = 1

PC=0x001A: CMP R0, R1
  → Compare 5 with 1
  → Flags: Z=0, N=0 (5 > 1)

PC=0x001B: BLE base_case
  → Not taken (5 > 1)

PC=0x001D: MOV R2, R0
  → R2 = 5

PC=0x001E: DEC R0
  → R0 = 4 (argument for next call)

PC=0x001F: CALL factorial
  → Stack: [0x0008][0x0000][0x0000][0x0021]
  → SP = 0xDFFC
  → PC = 0x0016 (recursive call to factorial)
```

**Stack State After First Recursive CALL:**
```
Address    Value        Description
─────────────────────────────────────────
0xE000     [empty]
0xDFFF     0x0008       Return to main
0xDFFE     0x0000       factorial(5): saved R1
0xDFFD     0x0000       factorial(5): saved R2
0xDFFC     0x0021       Return to factorial(5)
           ↑ SP
```

---

#### **Phase 3: factorial(4) - Second Call**

```
Registers:
  R0 = 4 (argument)
  R2 = 5 (from previous frame)
  SP = 0xDFFC

PC=0x0016: PUSH R1
PC=0x0017: PUSH R2
  → Stack: [0x0008][0x0000][0x0000][0x0021][0x0000][0x0000]
  → SP = 0xDFFA

[Same check, R0=4 > 1, so recursive case]

PC=0x001D: MOV R2, R0
  → R2 = 4

PC=0x001E: DEC R0
  → R0 = 3

PC=0x001F: CALL factorial
  → SP = 0xDFF7
```

**Stack State:**
```
Address    Value        Description
─────────────────────────────────────────
0xDFFF     0x0008       Return to main
0xDFFE     0x0000       factorial(5): saved R1
0xDFFD     0x0000       factorial(5): saved R2
0xDFFC     0x0021       Return to factorial(5)
0xDFFB     0x0000       factorial(4): saved R1
0xDFFA     0x0000       factorial(4): saved R2
0xDFF9     0x0021       Return to factorial(4)
           ↑ SP
```

---

#### **Phase 4: Deeper Recursion**

The pattern continues for factorial(3), factorial(2), and factorial(1):

**factorial(3) call:**
```
- R0 = 3, R2 = 4
- Stack grows to 0xDFF4
```

**factorial(2) call:**
```
- R0 = 2, R2 = 3
- Stack grows to 0xDFF1
```

**factorial(1) call (BASE CASE):**
```
- R0 = 1, R2 = 2
- Stack grows to 0xDFEE
```

**Maximum Stack Depth:**
```
Address    Value        Description
─────────────────────────────────────────
0xDFFF     0x0008       Return to main
0xDFFE     0x0000       factorial(5): saved R1
0xDFFD     0x0000       factorial(5): saved R2
0xDFFC     0x0021       Return to factorial(5)
0xDFFB     0x0000       factorial(4): saved R1
0xDFFA     0x0000       factorial(4): saved R2
0xDFF9     0x0021       Return to factorial(4)
0xDFF8     0x0000       factorial(3): saved R1
0xDFF7     0x0000       factorial(3): saved R2
0xDFF6     0x0021       Return to factorial(3)
0xDFF5     0x0000       factorial(2): saved R1
0xDFF4     0x0000       factorial(2): saved R2
0xDFF3     0x0021       Return to factorial(2)
0xDFF2     0x0000       factorial(1): saved R1
0xDFF1     0x0000       factorial(1): saved R2
0xDFF0     0x0021       Return to factorial(1)
           ↑ SP (0xDFF0)
```

---

#### **Phase 5: Base Case - factorial(1)**

```
PC=0x0016: PUSH R1
PC=0x0017: PUSH R2
  → SP = 0xDFEE

PC=0x0018: LDI R1, 1
  → R1 = 1

PC=0x001A: CMP R0, R1
  → Compare 1 with 1
  → Flags: Z=1 (equal)

PC=0x001B: BLE base_case
  → TAKEN! Jump to 0x0025

PC=0x0025: LDI R0, 1
  → R0 = 1 (return value)

PC=0x0026: POP R2
  → SP = 0xDFEF

PC=0x0027: POP R1
  → SP = 0xDFF0

PC=0x0028: RET
  → PC = Memory[SP] = 0x0021 (return to factorial(2))
  → SP = 0xDFF1
  → R0 = 1 (factorial(1) = 1)
```

---

#### **Phase 6: Unwinding - factorial(2) resumes**

```
PC=0x0021: MUL R0, R2
  → R0 = 1 * 2 = 2
  → R2 still contains 2 from before the call

PC=0x0022: JMP factorial_return
  → PC = 0x0026

PC=0x0026: POP R2
PC=0x0027: POP R1
  → SP = 0xDFF3

PC=0x0028: RET
  → PC = 0x0021 (return to factorial(3))
  → SP = 0xDFF4
  → R0 = 2 (factorial(2) = 2)
```

---

#### **Phase 7: Continued Unwinding**

**factorial(3) resumes:**
```
PC=0x0021: MUL R0, R2
  → R0 = 2 * 3 = 6
  → Returns 6
```

**factorial(4) resumes:**
```
PC=0x0021: MUL R0, R2
  → R0 = 6 * 4 = 24
  → Returns 24
```

**factorial(5) resumes:**
```
PC=0x0021: MUL R0, R2
  → R0 = 24 * 5 = 120
  → Returns 120
```

---

#### **Phase 8: Return to main()**

```
PC=0x0021: MUL R0, R2
  → R0 = 24 * 5 = 120

PC=0x0022: JMP factorial_return

PC=0x0026: POP R2
PC=0x0027: POP R1
  → SP = 0xDFFF

PC=0x0028: RET
  → PC = 0x0008 (return to main)
  → SP = 0xE000
  → R0 = 120 (final result!)

Stack is now empty again: SP = 0xE000
```

---

#### **Phase 9: main() prints result**

```
PC=0x0008: MOV R1, R0
  → R1 = 120

PC=0x0009: LDI R0, msg_result
  → R0 = 0xDFF5

PC=0x000B: ST [0xF802], R0
  → Print "Result: "

PC=0x000D: MOV R0, R1
  → R0 = 120

PC=0x000E: ST [0xF801], R0
  → Print "120"

PC=0x0010: LDI R0, 10
  → R0 = 10 (newline)

PC=0x0012: ST [0xF800], R0
  → Print '\n'

PC=0x0014: HALT
  → Execution stops
```

---

## Call Stack Visualization

```
Recursion Depth and Stack Growth:

main()
  │
  ├─ CALL factorial(5)      SP: 0xDFFF
      │
      ├─ CALL factorial(4)  SP: 0xDFF9
          │
          ├─ CALL factorial(3)  SP: 0xDFF3
              │
              ├─ CALL factorial(2)  SP: 0xDFED
                  │
                  └─ CALL factorial(1)  SP: 0xDFE7
                      │
                      └─ BASE CASE: return 1
                  │
                  ← return 1 * 2 = 2
              │
              ← return 2 * 3 = 6
          │
          ← return 6 * 4 = 24
      │
      ← return 24 * 5 = 120
  │
  ← result = 120
```

---

## Key Concepts Demonstrated

### 1. Function Call Mechanism

**CALL instruction:**
- Pushes return address (PC) onto stack
- Jumps to function address
- Stack Pointer decrements

**RET instruction:**
- Pops return address from stack into PC
- Stack Pointer increments
- Execution continues at saved address

### 2. Stack Frame Management

Each function creates a frame containing:
- **Return address** (saved by CALL)
- **Saved registers** (saved by prologue)
- **Local variables** (if needed)

### 3. Register Preservation

**Prologue** (function entry):
```assembly
PUSH R1          ; Save registers
PUSH R2
```

**Epilogue** (function exit):
```assembly
POP R2           ; Restore in reverse order
POP R1
RET
```

### 4. Recursion Implementation

**Base case check:**
```assembly
CMP R0, R1       ; Check if n <= 1
BLE base_case    ; If so, return 1
```

**Recursive case:**
```assembly
MOV R2, R0       ; Save n
DEC R0           ; Prepare n-1
CALL factorial   ; Recursive call
MUL R0, R2       ; Multiply result by n
```

### 5. Stack Growth Pattern

```
Maximum stack usage for factorial(5):
- 5 nested calls
- Each call uses 3 words (ret_addr + 2 saved registers)
- Total: 5 × 3 = 15 words
- Stack grows from 0xE000 to 0xDFF1 (15 words)
```

---

## Memory Efficiency

### Space Complexity
- **Stack space**: O(n) for n recursive calls
- **Each frame**: 3 words
- **factorial(5)**: 15 words maximum

### Time Complexity
- **Recursive calls**: O(n)
- **Total instructions**: ~20 instructions per call
- **factorial(5)**: ~100 total instructions

---

## Summary

This example demonstrates:

1.  **Memory Layout**: Code, data, and stack segments clearly separated
2. **Function Calls**: CALL/RET mechanism with return address on stack
3.  **Recursion**: Multiple stack frames for nested calls
4.  **Stack Management**: Proper prologue/epilogue for register preservation
5.  **Calling Convention**: Arguments in R0, return value in R0
6.  **Base Case**: Termination condition prevents infinite recursion
7.  **Unwinding**: Stack frames removed in reverse order (LIFO)

The factorial(5) computation successfully demonstrates all aspects of function calling and recursion on the SimpleCPU16 architecture!

---

*End of Execution Trace*