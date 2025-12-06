#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_LABELS 256
#define MAX_LINE_LENGTH 256
#define MAX_TOKENS 10

// Label structure
typedef struct {
    char name[64];
    uint16_t address;
} Label;

// Assembler state
typedef struct {
    Label labels[MAX_LABELS];
    int label_count;
    uint16_t current_address;
    uint16_t* output;
    int output_size;
    int output_capacity;
} Assembler;

// Token types
typedef enum {
    TOKEN_LABEL,
    TOKEN_INSTRUCTION,
    TOKEN_REGISTER,
    TOKEN_IMMEDIATE,
    TOKEN_ADDRESS,
    TOKEN_DIRECTIVE,
    TOKEN_STRING,
    TOKEN_COMMA,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_UNKNOWN
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    char value[128];
    int num_value;
} Token;

// Function prototypes
void asm_init(Assembler* asm_state);
void asm_free(Assembler* asm_state);
bool asm_assemble_file(Assembler* asm_state, const char* input_file, const char* output_file);
bool asm_assemble_line(Assembler* asm_state, const char* line, int pass);
void asm_add_label(Assembler* asm_state, const char* name, uint16_t address);
int asm_find_label(Assembler* asm_state, const char* name);
void asm_emit_word(Assembler* asm_state, uint16_t word);
int asm_tokenize(const char* line, Token tokens[], int max_tokens);
int asm_parse_register(const char* str);
int asm_parse_number(const char* str);
uint16_t asm_encode_instruction(const char* mnemonic, Token operands[], int operand_count, Assembler* asm_state);

#endif // ASSEMBLER_H
