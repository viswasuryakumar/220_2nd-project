#include "assembler.h"
#include "../emulator/cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Initializing assembler state
void asm_init(Assembler* asm_state) {
    asm_state->label_count = 0;
    asm_state->current_address = 0;
    asm_state->output_capacity = 1024;
    asm_state->output_size = 0;
    asm_state->output = (uint16_t*)malloc(asm_state->output_capacity * sizeof(uint16_t));
}

// Freeing assembler resources
void asm_free(Assembler* asm_state) {
    if (asm_state->output) {
        free(asm_state->output);
        asm_state->output = NULL;
    }
}

// Adding label to symbol table
void asm_add_label(Assembler* asm_state, const char* name, uint16_t address) {
    if (asm_state->label_count >= MAX_LABELS) {
        fprintf(stderr, "Error: Too many labels\n");
        return;
    }
    
    strncpy(asm_state->labels[asm_state->label_count].name, name, 63);
    asm_state->labels[asm_state->label_count].name[63] = '\0';
    asm_state->labels[asm_state->label_count].address = address;
    asm_state->label_count++;
}

// Finding label address
int asm_find_label(Assembler* asm_state, const char* name) {
    for (int i = 0; i < asm_state->label_count; i++) {
        if (strcmp(asm_state->labels[i].name, name) == 0) {
            return asm_state->labels[i].address;
        }
    }
    return -1;
}

// Emitting word to output
void asm_emit_word(Assembler* asm_state, uint16_t word) {
    if (asm_state->output_size >= asm_state->output_capacity) {
        asm_state->output_capacity *= 2;
        asm_state->output = (uint16_t*)realloc(asm_state->output, 
                                                asm_state->output_capacity * sizeof(uint16_t));
    }
    
    asm_state->output[asm_state->output_size++] = word;
    asm_state->current_address++;
}

// Parsing register name
int asm_parse_register(const char* str) {
    if (str[0] != 'R' && str[0] != 'r') {
        if (strcasecmp(str, "SP") == 0) return REG_SP;
        return -1;
    }

    // Check that character after 'R' is a digit
    if (!isdigit(str[1])) {
        return -1;  // Not a valid register (e.g., "RET", "READ", etc.)
    }

    int reg_num = atoi(str + 1);
    if (reg_num >= 0 && reg_num < NUM_REGISTERS) {
        return reg_num;
    }

    return -1;
}

// Parsing numeric value (decimal, hex, or char)
int asm_parse_number(const char* str) {
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        return (int)strtol(str + 2, NULL, 16);
    } else if (str[0] == '\'') {
        if (str[2] == '\'') {
            return (int)str[1];
        }
    }
    return atoi(str);
}

// Tokenizing assembly line
int asm_tokenize(const char* line, Token tokens[], int max_tokens) {
    int token_count = 0;
    char buffer[MAX_LINE_LENGTH];
    strncpy(buffer, line, MAX_LINE_LENGTH - 1);
    buffer[MAX_LINE_LENGTH - 1] = '\0';
    
    // Removing comments
    char* comment = strchr(buffer, ';');
    if (comment) *comment = '\0';
    
    // Removing trailing whitespace
    int len = strlen(buffer);
    while (len > 0 && isspace(buffer[len-1])) {
        buffer[--len] = '\0';
    }
    
    if (len == 0) return 0;
    
    char* ptr = buffer;
    while (*ptr && token_count < max_tokens) {
        // Skipping whitespace
        while (*ptr && isspace(*ptr)) ptr++;
        if (!*ptr) break;
        
        // Checking for string literal
        if (*ptr == '"') {
            ptr++;
            char* start = ptr;
            while (*ptr && *ptr != '"') ptr++;
            int str_len = ptr - start;
            strncpy(tokens[token_count].value, start, str_len);
            tokens[token_count].value[str_len] = '\0';
            tokens[token_count].type = TOKEN_STRING;
            token_count++;
            if (*ptr == '"') ptr++;
            continue;
        }
        
        // Checking for special characters
        if (*ptr == ',') {
            tokens[token_count].type = TOKEN_COMMA;
            tokens[token_count].value[0] = ',';
            tokens[token_count].value[1] = '\0';
            token_count++;
            ptr++;
            continue;
        }
        
        if (*ptr == '[') {
            tokens[token_count].type = TOKEN_LBRACKET;
            tokens[token_count].value[0] = '[';
            tokens[token_count].value[1] = '\0';
            token_count++;
            ptr++;
            continue;
        }
        
        if (*ptr == ']') {
            tokens[token_count].type = TOKEN_RBRACKET;
            tokens[token_count].value[0] = ']';
            tokens[token_count].value[1] = '\0';
            token_count++;
            ptr++;
            continue;
        }
        
        // Reading word token
        char* start = ptr;
        while (*ptr && !isspace(*ptr) && *ptr != ',' && *ptr != '[' && *ptr != ']' && *ptr != ';') {
            ptr++;
        }
        
        int word_len = ptr - start;
        if (word_len == 0) continue;
        
        strncpy(tokens[token_count].value, start, word_len);
        tokens[token_count].value[word_len] = '\0';
        
        // Determining token type
        if (tokens[token_count].value[word_len-1] == ':') {
            tokens[token_count].value[word_len-1] = '\0';
            tokens[token_count].type = TOKEN_LABEL;
        } else if (tokens[token_count].value[0] == '.') {
            tokens[token_count].type = TOKEN_DIRECTIVE;
        } else if (asm_parse_register(tokens[token_count].value) >= 0) {
            tokens[token_count].type = TOKEN_REGISTER;
            tokens[token_count].num_value = asm_parse_register(tokens[token_count].value);
        } else if (isdigit(tokens[token_count].value[0]) || 
                   tokens[token_count].value[0] == '\'' ||
                   tokens[token_count].value[0] == '-') {
            tokens[token_count].type = TOKEN_IMMEDIATE;
            tokens[token_count].num_value = asm_parse_number(tokens[token_count].value);
        } else {
            tokens[token_count].type = TOKEN_INSTRUCTION;
        }
        
        token_count++;
    }
    
    return token_count;
}

// Encoding instruction
uint16_t asm_encode_instruction(const char* mnemonic, Token operands[], int operand_count, Assembler* asm_state) {
    // Suppress unused parameter warnings
    (void)operand_count;
    (void)asm_state;

    uint8_t rd = 0, rs = 0;
    
    // NOP
    if (strcasecmp(mnemonic, "NOP") == 0) {
        return (OP_NOP << 12);
    }
    
    // HALT
    else if (strcasecmp(mnemonic, "HALT") == 0) {
        return (OP_HALT << 12);
    }
    
    // MOV Rd, Rs
    else if (strcasecmp(mnemonic, "MOV") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;  // Skip comma
        return (OP_MOVE << 12) | (rd << 9) | (rs << 6);
    }
    
    // LDI Rd, immediate
    else if (strcasecmp(mnemonic, "LDI") == 0) {
        rd = operands[0].num_value;
        // Emitting immediate value after instruction
        return (OP_LOAD << 12) | (rd << 9) | LOAD_IMM;
    }
    
    // LD Rd, [address] or LD Rd, [Rs]
    else if (strcasecmp(mnemonic, "LD") == 0) {
        rd = operands[0].num_value;
        // Checking if indirect [Rs] or direct [address]
        // Token layout: LD R1, [ R5 ] -> operands[3] is the register/address inside brackets
        if (operands[3].type == TOKEN_REGISTER) {
            rs = operands[3].num_value;
            return (OP_LOAD << 12) | (rd << 9) | (rs << 6) | LOAD_IND;
        } else {
            return (OP_LOAD << 12) | (rd << 9) | LOAD_DIR;
        }
    }
    
    // ST [address], Rs or ST [Rd], Rs
    else if (strcasecmp(mnemonic, "ST") == 0) {
        if (operands[1].type == TOKEN_REGISTER) {
            rd = operands[1].num_value;
            rs = operands[4].num_value;  // After comma
            return (OP_STORE << 12) | (rd << 9) | (rs << 6) | STORE_IND;
        } else {
            rs = operands[4].num_value;  // Register after comma
            return (OP_STORE << 12) | (rs << 6) | STORE_DIR;
        }
    }
    
    // Arithmetic instructions
    else if (strcasecmp(mnemonic, "ADD") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_ARITH << 12) | (rd << 9) | (rs << 6) | ARITH_ADD;
    }
    else if (strcasecmp(mnemonic, "SUB") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_ARITH << 12) | (rd << 9) | (rs << 6) | ARITH_SUB;
    }
    else if (strcasecmp(mnemonic, "MUL") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_ARITH << 12) | (rd << 9) | (rs << 6) | ARITH_MUL;
    }
    else if (strcasecmp(mnemonic, "DIV") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_ARITH << 12) | (rd << 9) | (rs << 6) | ARITH_DIV;
    }
    else if (strcasecmp(mnemonic, "INC") == 0) {
        rd = operands[0].num_value;
        return (OP_ARITH << 12) | (rd << 9) | ARITH_INC;
    }
    else if (strcasecmp(mnemonic, "DEC") == 0) {
        rd = operands[0].num_value;
        return (OP_ARITH << 12) | (rd << 9) | ARITH_DEC;
    }
    else if (strcasecmp(mnemonic, "ADDI") == 0) {
        rd = operands[0].num_value;
        return (OP_ARITH << 12) | (rd << 9) | ARITH_ADDI;
    }
    else if (strcasecmp(mnemonic, "SUBI") == 0) {
        rd = operands[0].num_value;
        return (OP_ARITH << 12) | (rd << 9) | ARITH_SUBI;
    }
    
    // Logical instructions
    else if (strcasecmp(mnemonic, "AND") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_LOGIC << 12) | (rd << 9) | (rs << 6) | LOGIC_AND;
    }
    else if (strcasecmp(mnemonic, "OR") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_LOGIC << 12) | (rd << 9) | (rs << 6) | LOGIC_OR;
    }
    else if (strcasecmp(mnemonic, "XOR") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_LOGIC << 12) | (rd << 9) | (rs << 6) | LOGIC_XOR;
    }
    else if (strcasecmp(mnemonic, "NOT") == 0) {
        rd = operands[0].num_value;
        return (OP_LOGIC << 12) | (rd << 9) | LOGIC_NOT;
    }
    
    // Shift instructions
    else if (strcasecmp(mnemonic, "SHL") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_SHIFT << 12) | (rd << 9) | (rs << 6) | SHIFT_LEFT;
    }
    else if (strcasecmp(mnemonic, "SHR") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_SHIFT << 12) | (rd << 9) | (rs << 6) | SHIFT_RIGHT;
    }
    else if (strcasecmp(mnemonic, "SAR") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_SHIFT << 12) | (rd << 9) | (rs << 6) | SHIFT_ARITH;
    }
    
    // Compare
    else if (strcasecmp(mnemonic, "CMP") == 0) {
        rd = operands[0].num_value;
        rs = operands[2].num_value;
        return (OP_CMP << 12) | (rd << 9) | (rs << 6);
    }
    
    // Stack operations
    else if (strcasecmp(mnemonic, "PUSH") == 0) {
        rs = operands[0].num_value;
        return (OP_STACK << 12) | (rs << 6) | STACK_PUSH;
    }
    else if (strcasecmp(mnemonic, "POP") == 0) {
        rd = operands[0].num_value;
        return (OP_STACK << 12) | (rd << 9) | STACK_POP;
    }
    
    // Branches
    else if (strcasecmp(mnemonic, "BEQ") == 0) {
        return (OP_BRANCH << 12) | BRANCH_EQ;
    }
    else if (strcasecmp(mnemonic, "BNE") == 0) {
        return (OP_BRANCH << 12) | BRANCH_NE;
    }
    else if (strcasecmp(mnemonic, "BGT") == 0) {
        return (OP_BRANCH << 12) | BRANCH_GT;
    }
    else if (strcasecmp(mnemonic, "BLT") == 0) {
        return (OP_BRANCH << 12) | BRANCH_LT;
    }
    else if (strcasecmp(mnemonic, "BGE") == 0) {
        return (OP_BRANCH << 12) | BRANCH_GE;
    }
    else if (strcasecmp(mnemonic, "BLE") == 0) {
        return (OP_BRANCH << 12) | BRANCH_LE;
    }
    else if (strcasecmp(mnemonic, "BCS") == 0) {
        return (OP_BRANCH << 12) | BRANCH_CS;
    }
    else if (strcasecmp(mnemonic, "BCC") == 0) {
        return (OP_BRANCH << 12) | BRANCH_CC;
    }
    
    // Jump
    else if (strcasecmp(mnemonic, "JMP") == 0) {
        return (OP_JUMP << 12);
    }
    
    // Call and Return
    else if (strcasecmp(mnemonic, "CALL") == 0) {
        return (OP_CALL << 12);
    }
    else if (strcasecmp(mnemonic, "RET") == 0) {
        return (OP_RET << 12);
    }
    
    fprintf(stderr, "Unknown instruction: %s\n", mnemonic);
    return 0;
}

// Assembling single line
bool asm_assemble_line(Assembler* asm_state, const char* line, int pass) {
    Token tokens[MAX_TOKENS];
    int token_count = asm_tokenize(line, tokens, MAX_TOKENS);

    if (token_count == 0) return true;
    
    int token_idx = 0;
    
    // Processing label if present
    if (tokens[token_idx].type == TOKEN_LABEL) {
        if (pass == 1) {
            asm_add_label(asm_state, tokens[token_idx].value, asm_state->current_address);
        }
        token_idx++;
        if (token_idx >= token_count) return true;
    }
    
    // Processing directive
    if (tokens[token_idx].type == TOKEN_DIRECTIVE) {
        if (strcasecmp(tokens[token_idx].value, ".ORG") == 0) {
            uint16_t org_addr = asm_parse_number(tokens[token_idx + 1].value);
            asm_state->current_address = org_addr;
            if (pass == 2) {
                asm_state->output_size = org_addr;
            }
        }
        else if (strcasecmp(tokens[token_idx].value, ".WORD") == 0) {
            if (pass == 2) {
                for (int i = token_idx + 1; i < token_count; i++) {
                    if (tokens[i].type != TOKEN_COMMA) {
                        int value;
                        if (tokens[i].type == TOKEN_IMMEDIATE) {
                            value = tokens[i].num_value;
                        } else {
                            // Could be a label
                            value = asm_find_label(asm_state, tokens[i].value);
                            if (value == -1) value = 0;
                        }
                        asm_emit_word(asm_state, (uint16_t)value);
                    }
                }
            } else {
                // Counting words in pass 1
                for (int i = token_idx + 1; i < token_count; i++) {
                    if (tokens[i].type != TOKEN_COMMA) {
                        asm_state->current_address++;
                    }
                }
            }
        }
        else if (strcasecmp(tokens[token_idx].value, ".STRING") == 0 ||
                 strcasecmp(tokens[token_idx].value, ".ASCIIZ") == 0) {
            if (pass == 2 && token_idx + 1 < token_count) {
                const char* str = tokens[token_idx + 1].value;
                // Pack two characters per word (low byte, high byte)
                for (int i = 0; str[i]; i += 2) {
                    uint16_t word = (str[i] & 0xFF);
                    if (str[i + 1] != '\0') {
                        word |= ((uint16_t)(str[i + 1] & 0xFF)) << 8;
                    }
                    asm_emit_word(asm_state, word);
                }
                asm_emit_word(asm_state, 0);  // Null terminator
            } else {
                const char* str = tokens[token_idx + 1].value;
                int str_len = strlen(str);
                // Calculate words needed: (length + 1) / 2, plus null terminator
                asm_state->current_address += (str_len + 1) / 2 + 1;
            }
        }
        return true;
    }
    
    // Processing instruction
    if (tokens[token_idx].type == TOKEN_INSTRUCTION) {
        const char* mnemonic = tokens[token_idx].value;
        Token* operands = &tokens[token_idx + 1];
        int operand_count = token_count - token_idx - 1;

        if (pass == 2) {
            uint16_t instruction = asm_encode_instruction(mnemonic, operands, operand_count, asm_state);
            asm_emit_word(asm_state, instruction);
            
            // Checking if we need to emit immediate/address
            if (strcasecmp(mnemonic, "LDI") == 0 || strcasecmp(mnemonic, "ADDI") == 0 || 
                strcasecmp(mnemonic, "SUBI") == 0) {
                int value = operands[2].num_value;  // After comma
                if (operands[2].type != TOKEN_IMMEDIATE && operands[2].type != TOKEN_REGISTER) {
                    value = asm_find_label(asm_state, operands[2].value);
                    if (value == -1) {
                        fprintf(stderr, "Undefined label: %s\n", operands[2].value);
                        value = 0;
                    }
                }
                asm_emit_word(asm_state, (uint16_t)value);
            }
            else if (strcasecmp(mnemonic, "LD") == 0 && operands[3].type != TOKEN_REGISTER) {
                // Token layout: LD R1, [ 0x1000 ] -> operands[3] is the address
                int addr = asm_parse_number(operands[3].value);
                asm_emit_word(asm_state, (uint16_t)addr);
            }
            else if (strcasecmp(mnemonic, "ST") == 0 && operands[1].type != TOKEN_REGISTER) {
                int addr = asm_parse_number(operands[1].value);
                asm_emit_word(asm_state, (uint16_t)addr);
            }
            else if (strcasecmp(mnemonic, "JMP") == 0 || strcasecmp(mnemonic, "CALL") == 0 ||
                     strncasecmp(mnemonic, "B", 1) == 0) {  // Branch instructions
                int addr;
                if (operands[0].type == TOKEN_IMMEDIATE) {
                    addr = operands[0].num_value;
                } else {
                    addr = asm_find_label(asm_state, operands[0].value);
                    if (addr == -1) {
                        fprintf(stderr, "Undefined label: %s\n", operands[0].value);
                        addr = 0;
                    }
                }
                asm_emit_word(asm_state, (uint16_t)addr);
            }
        } else {
            // Pass 1: counting instruction size
            asm_state->current_address++;
            // Checking for multi-word instructions
            if (strcasecmp(mnemonic, "LDI") == 0 || strcasecmp(mnemonic, "ADDI") == 0 ||
                strcasecmp(mnemonic, "SUBI") == 0 || strcasecmp(mnemonic, "JMP") == 0 ||
                strcasecmp(mnemonic, "CALL") == 0 || strncasecmp(mnemonic, "B", 1) == 0) {
                asm_state->current_address++;
            }
            else if (strcasecmp(mnemonic, "LD") == 0 && operands[3].type != TOKEN_REGISTER) {
                asm_state->current_address++;
            }
            else if (strcasecmp(mnemonic, "ST") == 0 && operands[1].type != TOKEN_REGISTER) {
                asm_state->current_address++;
            }
        }
    }
    
    return true;
}

// Assembling file (two-pass assembler)
bool asm_assemble_file(Assembler* asm_state, const char* input_file, const char* output_file) {
    FILE* fp = fopen(input_file, "r");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open input file %s\n", input_file);
        return false;
    }
    
    char line[MAX_LINE_LENGTH];
    
    // Pass 1: Collecting labels
    printf("Pass 1: Collecting labels...\n");
    asm_state->current_address = 0;
    while (fgets(line, sizeof(line), fp)) {
        asm_assemble_line(asm_state, line, 1);
    }
    
    printf("Found %d labels\n", asm_state->label_count);
    
    // Pass 2: Generating code
    printf("Pass 2: Generating code...\n");
    fseek(fp, 0, SEEK_SET);
    asm_state->current_address = 0;
    
    while (fgets(line, sizeof(line), fp)) {
        asm_assemble_line(asm_state, line, 2);
    }
    
    fclose(fp);
    
    // Writing output file
    FILE* out = fopen(output_file, "wb");
    if (!out) {
        fprintf(stderr, "Error: Cannot open output file %s\n", output_file);
        return false;
    }
    
    fwrite(asm_state->output, sizeof(uint16_t), asm_state->output_size, out);
    fclose(out);
    
    printf("Assembly complete: %d words written to %s\n", asm_state->output_size, output_file);
    return true;
}
