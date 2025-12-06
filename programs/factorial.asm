; Recursive Factorial Program for SimpleCPU16
; ============================================
; This program demonstrates recursion by computing factorial(5) = 120
;
; CALLING CONVENTION:
; - Argument passed in R0
; - Return value returned in R0
; - R1-R3 are caller-saved (can be modified by callee)
; - R4-R6 are callee-saved (must be preserved by callee)
; - R7 is stack pointer (SP)
;
; MEMORY LAYOUT:
; 0x0000 - 0x00XX : Code section (main, factorial function)
; 0xDFE0 - 0xDFFF : Data section (strings, constants)
; 0xE000          : Stack starts here (grows downward)

.ORG 0x0000

; ====================
; MAIN PROGRAM
; ====================
main:
    ; Print startup message
    LDI R0, msg_computing
    ST [0xF802], R0           ; Print "Computing factorial..."

    ; Call factorial(5)
    LDI R0, 5                 ; Argument: n = 5
    CALL factorial            ; Call factorial function
    ; Return value is in R0

    ; Save result for printing
    MOV R1, R0                ; R1 = result (120)

    ; Print result message
    LDI R0, msg_result
    ST [0xF802], R0           ; Print "Result: "

    ; Print the result
    MOV R0, R1                ; R0 = result
    ST [0xF801], R0           ; Print integer result

    ; Print newline
    LDI R0, 10                ; ASCII newline
    ST [0xF800], R0           ; Print character

    HALT

; ====================
; FACTORIAL FUNCTION
; ====================
; Computes factorial recursively: factorial(n) = n * factorial(n-1)
; Base case: factorial(0) = factorial(1) = 1
;
; Arguments:  R0 = n (input value)
; Returns:    R0 = n! (factorial result)
; Uses:       R1 (temporary), R2 (saved n value)
;
; STACK FRAME LAYOUT (grows downward):
; [SP+2]: Return address (pushed by CALL)
; [SP+1]: Saved R2
; [SP+0]: Saved R1 (current SP points here after prologue)

factorial:
    ; --- PROLOGUE: Save registers we'll use ---
    PUSH R1                   ; Save R1
    PUSH R2                   ; Save R2
    ; Stack now: [ret_addr][R1][R2] <- SP

    ; --- CHECK BASE CASE: if (n <= 1) return 1 ---
    LDI R1, 1                 ; R1 = 1 for comparison
    CMP R0, R1                ; Compare n with 1
    BLE base_case             ; If n <= 1, jump to base case

    ; --- RECURSIVE CASE: return n * factorial(n-1) ---
    MOV R2, R0                ; R2 = n (save original n)

    ; Prepare argument for recursive call: n-1
    DEC R0                    ; R0 = n - 1

    ; Make recursive call
    CALL factorial            ; Call factorial(n-1)
    ; R0 now contains factorial(n-1)

    ; Multiply: R0 = n * factorial(n-1)
    MUL R0, R2                ; R0 = factorial(n-1) * n

    ; Jump to epilogue
    JMP factorial_return

base_case:
    ; --- BASE CASE: return 1 ---
    LDI R0, 1                 ; Return value = 1

factorial_return:
    ; --- EPILOGUE: Restore registers and return ---
    POP R2                    ; Restore R2
    POP R1                    ; Restore R1
    RET                       ; Return to caller

; ====================
; DATA SECTION
; ====================
.ORG 0xDFE0

msg_computing:
    .STRING "Computing factorial of 5..."

msg_result:
    .STRING "Result: "