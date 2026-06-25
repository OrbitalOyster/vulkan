# Generic Makefile

# Working directories
SRC_DIR := src
INCLUDE_DIR := include
OBJ_DIR := obj
BIN_DIR := bin

# Executable name
EXE := dongle

# Actual language
CC := clang
STANDART := -std=c17
# Compiler options
OUTPUT := $(BIN_DIR)/$(EXE)
OPTIMIZATION := -O2
WARNINGS := -Wall -Wextra -Wpedantic -Werror
# Debugging info
DEBUG := -ggdb3
# Preprocessor flags (example -DDEBUG -DLOG)
DFLAGS :=
# All above combined
CFLAGS := $(STANDART) $(OPTIMIZATION) $(WARNINGS) $(DFLAGS) $(DEBUG) -I $(INCLUDE_DIR)

# Linker libraries (example -lm)
LDLIBS := -lglfw -lvulkan -lGL

# All source subdirectories
SRC_SDIRS := $(SRC_DIR)/ $(wildcard $(SRC_DIR)/*/) $(wildcard $(SRC_DIR)/**/*/)
INCLUDE_SDIRS := $(INCLUDE_DIR)/ $(wildcard $(INCLUDE_DIR)/*/) $(wildcard $(INCLUDE_DIR)/**/*/)

# All .c files
C_FILES := $(foreach d, $(SRC_SDIRS), $(wildcard $(d)*.c))

# All .h files
H_FILES := $(foreach d, $(INCLUDE_SDIRS), $(wildcard $(d)*.h))

# All .o files
OBJS := $(patsubst $(SRC_DIR)%, $(OBJ_DIR)%, $(C_FILES:.c=.o))
OBJ_SDIRS := $(patsubst $(SRC_DIR)%, $(OBJ_DIR)%, $(SRC_SDIRS))

# All .d files
DEPS := $(OBJS:.o=.d)

# Final result
all: $(OUTPUT)

# Object subdirectories and files
$(OUTPUT): $(OBJ_SDIRS) $(OBJS) $(BIN_DIR) $(INCLUDE_DIR)
	$(CC) $(OBJS) $(LDLIBS) -o $(OUTPUT)

$(OBJ_SDIRS):
	mkdir -p $(OBJ_SDIRS)

# Include directory (might be empty)
$(INCLUDE_DIR):
		mkdir -p $(INCLUDE_DIR)

# Executable directory
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile all .c files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(INCLUDE_DIR) Makefile
	$(CC) $(CFLAGS) -MMD -MP -c -o $@ $<

# Header dependencies
-include $(DEPS)

# Run executable
run: $(OUTPUT)
	./$(OUTPUT)

# Format source files
format:
	clang-format $(C_FILES) $(H_FILES) -i

# Clear working directory
clean:
	-rm $(OBJ_DIR)/* -r
	-rmdir $(OBJ_DIR)
	-rm $(OUTPUT)
	-rmdir $(BIN_DIR)
	-rm compile_commands.json
	-rm .cache -r

.PHONY: all run format clean
