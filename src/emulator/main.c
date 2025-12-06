#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

void print_usage(const char* program_name) {
    printf("SimpleCPU16 Emulator\n");
    printf("Usage: %s <binary_file> [options]\n", program_name);
    printf("Options:\n");
    printf("  --trace         Enable instruction trace\n");
    printf("  --memdump FILE  Dump memory to file after execution\n");
    printf("  --help          Show this help message\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }
    
    const char* binary_file = NULL;
    bool trace = false;
    const char* memdump_file = NULL;
    
    // Parsing command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--trace") == 0) {
            trace = true;
        } else if (strcmp(argv[i], "--memdump") == 0) {
            if (i + 1 < argc) {
                memdump_file = argv[++i];
            }
        } else if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (binary_file == NULL) {
            binary_file = argv[i];
        }
    }
    
    if (binary_file == NULL) {
        fprintf(stderr, "Error: No binary file specified\n");
        print_usage(argv[0]);
        return 1;
    }
    
    // Loading binary file
    FILE* fp = fopen(binary_file, "rb");
    if (!fp) {
        fprintf(stderr, "Error: Cannot open binary file %s\n", binary_file);
        return 1;
    }
    
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    int word_count = file_size / sizeof(uint16_t);
    uint16_t* program = (uint16_t*)malloc(file_size);

    if (fread(program, sizeof(uint16_t), word_count, fp) != (size_t)word_count) {
        fprintf(stderr, "Error: Failed to read binary file\n");
        fclose(fp);
        free(program);
        return 1;
    }
    
    fclose(fp);
    
    // Initializing and running CPU
    printf("SimpleCPU16 Emulator v1.0\n");
    printf("==========================\n\n");
    
    CPU cpu;
    cpu_init(&cpu);
    cpu_load_program(&cpu, program, word_count, 0x0000);
    
    free(program);
    
    cpu_run(&cpu, trace);
    
    cpu_dump_registers(&cpu);
    
    if (memdump_file) {
        cpu_dump_memory(&cpu, memdump_file);
    }
    
    return 0;
}
