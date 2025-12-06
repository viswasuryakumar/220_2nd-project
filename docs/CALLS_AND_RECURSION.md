**SimpleCPU16 — Calls, Stack Frames and Recursion**

- **Code section**: starts at `0x0000` (where `.ORG 0x0000` programs are placed)
- **Data section**: this project uses `.ORG 0xDFE0` for strings and other constants
- **Stack**: starts at `0xE000` and grows downward (each word is 2 bytes)
- **Registers**:
  - `R0`: argument / return value
  - `R1`-`R3`: caller-saved temporaries
  - `R4`-`R6`: callee-saved registers
  - `R7`: stack pointer (SP)

Memory layout (high-level)
- 0x0000 — code
- 0xDFE0 — data (strings)
- 0xE000 — stack start (grows down)

Stack frame convention used in `programs/factorial.asm` and `programs/factorial_c_style.asm`:
- Caller: puts argument in `R0` then executes `CALL target`.
- `CALL` pushes the return address onto the stack and jumps to function.
- Callee prologue in example: `PUSH R1` then `PUSH R2` (saving used registers).
- Space used per `PUSH`/`POP` is one word (2 bytes) on this architecture.
- Epilogue: `POP` saved registers then `RET` (which pops return address and jumps).

Example: call `factorial(5)` — per-call stack activity (word-sized pushes)
Assume initial `SP = 0xE000`.

1) `main` does `LDI R0, 5` then `CALL factorial`.
   - `CALL` pushes return_address (1 word): `SP -> 0xDFFE` (ret_addr at [SP])
   - Enter `factorial`, prologue `PUSH R1`, `PUSH R2`:
     - After first PUSH (R1): `SP -> 0xDFFC` ([SP] = saved R1)
     - After second PUSH (R2): `SP -> 0xDFFA` ([SP] = saved R2)
   - Stack frame (top->bottom): [saved R2] [saved R1] [return_addr]

2) Within `factorial` for n=5, it saves R2=n, decrements R0 to 4 and `CALL factorial` again.
   - Another `CALL` pushes return_address (SP->0xDFF8), then prologue pushes R1 and R2 (SP->0xDFF4).

3) This continues until base case `n <= 1` is reached; then the base case sets `R0 = 1`, and functions return back up the chain.

Stack diagram for three nested calls (addresses shown as example words):

  Address    | Content
  -----------|------------------
  0xDFF4     | saved R2 (n=3)
  0xDFF6     | saved R1
  0xDFF8     | ret_addr -> return into caller for n=4
  0xDFFA     | saved R2 (n=4)
  0xDFFC     | saved R1
  0xDFFE     | ret_addr -> return into caller for n=5

When a `RET` executes in this convention:
- The function expects its epilogue to have restored callee-saved registers (or saved caller's registers depending on convention), then `RET` pops the return address and sets `PC` to it.

How recursion carries the computation:
- Each recursive call keeps its own copy of the saved registers (and implicitly the return address).
- The argument for the recursive call is passed in `R0` (modified before the `CALL`).
- After the recursive `CALL` returns, the caller uses the returned `R0` and its saved local (`R2` containing original `n`) to compute `n * factorial(n-1)`.

Mapping a small C function to this assembly pattern

C:
```
long factorial(long n) {
  if (n <= 1) return 1;
  return n * factorial(n - 1);
}
```
Assembly (conceptual mapping):
- argument in `R0`
- compare `R0` to 1 -> branch to base case
- save temp regs on stack
- compute `R0 = n - 1` and `CALL factorial`
- `MUL R0, saved_n` to combine result
- restore saved regs and `RET`

Files added/updated in this repo to show the above:
- `examples/factorial_c.c` — a host (native) C implementation and driver
- `programs/factorial_c_style.asm` — an assembly version following the C-call pattern
- `docs/CALLS_AND_RECURSION.md` — this document (explanation and diagrams)
- `Makefile` — updated to remove unused test targets (fibonacci, hello, timer)

Running and testing
- To run the native C example (build folder used for outputs):

```powershell
# compile and run host C example
gcc -o build/factorial_c.exe examples/factorial_c.c
.
build\factorial_c.exe
```

- To run the SimpleCPU16 assembly example with the project's assembler+emulator:

```powershell
# assemble and run on emulator
.\build\assembler.exe programs\factorial_c_style.asm -o build\factorial_c_style.bin
.\build\emulator.exe build\factorial_c_style.bin
```

