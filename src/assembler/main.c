#include "assembler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage(const char* program_name) {
    printf("SimpleCPU16 Assembler\n");
    printf("Usage: %s <input.asm> -o <output.bin>\n", program_name);
    printf("  <input.asm>   Assembly source file\n");
    printf("  -o <output>   Output binary file\n");
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* input_file = argv[1];
    const char* output_file = NULL;
    
    // Parsing command-line arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[++i];
        }
    }
    
    if (!output_file) {
        fprintf(stderr, "Error: No output file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    printf("SimpleCPU16 Assembler v1.0\n");
    printf("===========================\n\n");
    printf("Input:  %s\n", input_file);
    printf("Output: %s\n\n", output_file);
    
    Assembler asm_state;
    asm_init(&asm_state);
    
    bool success = asm_assemble_file(&asm_state, input_file, output_file);
    
    asm_free(&asm_state);
    
    if (success) {
        printf("\nAssembly successful!\n");
        return 0;
    } else {
        fprintf(stderr, "\nAssembly failed!\n");
        return 1;
    }
}
