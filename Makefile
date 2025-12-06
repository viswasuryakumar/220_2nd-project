CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
SRC_DIR = src
BUILD_DIR = build
PROG_DIR = programs

# Targets
EMULATOR = $(BUILD_DIR)/emulator
ASSEMBLER = $(BUILD_DIR)/assembler

# Source files
EMU_SRCS = $(SRC_DIR)/emulator/cpu.c $(SRC_DIR)/emulator/main.c
ASM_SRCS = $(SRC_DIR)/assembler/assembler.c $(SRC_DIR)/assembler/main.c

# Object files
EMU_OBJS = $(BUILD_DIR)/cpu.o $(BUILD_DIR)/emulator_main.o
ASM_OBJS = $(BUILD_DIR)/assembler.o $(BUILD_DIR)/assembler_main.o

# Default target
all: $(BUILD_DIR) $(EMULATOR) $(ASSEMBLER)

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build emulator
$(EMULATOR): $(EMU_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Build assembler
$(ASSEMBLER): $(ASM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile CPU module
$(BUILD_DIR)/cpu.o: $(SRC_DIR)/emulator/cpu.c $(SRC_DIR)/emulator/cpu.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile emulator main
$(BUILD_DIR)/emulator_main.o: $(SRC_DIR)/emulator/main.c $(SRC_DIR)/emulator/cpu.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile assembler module
$(BUILD_DIR)/assembler.o: $(SRC_DIR)/assembler/assembler.c $(SRC_DIR)/assembler/assembler.h $(SRC_DIR)/emulator/cpu.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Compile assembler main
$(BUILD_DIR)/assembler_main.o: $(SRC_DIR)/assembler/main.c $(SRC_DIR)/assembler/assembler.h
	$(CC) $(CFLAGS) -c -o $@ $<

# Assemble and run example programs
.PHONY: test_factorial test_all

test_factorial: all
	@echo "=== Assembling and running Recursive Factorial ==="
	$(ASSEMBLER) $(PROG_DIR)/factorial.asm -o $(BUILD_DIR)/factorial.bin
	$(EMULATOR) $(BUILD_DIR)/factorial.bin

test_all: test_factorial

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)

# Help
help:
	@echo "SimpleCPU16 Build System"
	@echo "========================"
	@echo ""
	@echo "Targets:"
	@echo "  all            - Build emulator and assembler"
	@echo "  test_factorial - Run Recursive Factorial (5! = 120)"
	@echo "  test_all       - Run all test programs (currently only factorial)"
	@echo "  clean          - Remove build artifacts"
	@echo "  help           - Show this help message"
