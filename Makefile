# ==========================================
# Compiler and Flags Configuration
# ==========================================
CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -Iinclude
LDFLAGS  :=

# ==========================================
# Target 1 Definition
# ==========================================
TARGET1       := lex
SRC_DIR1      := lex_src
BUILD_DIR1    := lex_build
SRCS1         := $(wildcard $(SRC_DIR1)/*.c)
OBJS1         := $(patsubst $(SRC_DIR1)/%.c, $(BUILD_DIR1)/%.o, $(SRCS1))

# ==========================================
# Target 2 Definition
# ==========================================
TARGET2       := vm
SRC_DIR2      := vm_src
BUILD_DIR2    := vm_build
SRCS2         := $(wildcard $(SRC_DIR2)/*.c)
OBJS2         := $(patsubst $(SRC_DIR2)/%.c, $(BUILD_DIR2)/%.o, $(SRCS2))

# ==========================================
# Phony Targets (Rules that don't match files)
# ==========================================
.PHONY: all clean lex vm

# Default rule builds both applications
all: lex vm

# Short-hand targets for building individual binaries
target1: $(TARGET1)
target2: $(TARGET2)

# ==========================================
# Linking Rules
# ==========================================

# Final executable for Target 1
$(TARGET1): $(OBJS1)
	@echo "Linking executable: $@"
	$(CC) $(OBJS1) -o $@ $(LDFLAGS)

# Final executable for Target 2
$(TARGET2): $(OBJS2)
	@echo "Linking executable: $@"
	$(CC) $(OBJS2) -o $@ $(LDFLAGS)

# ==========================================
# Compilation Rules (Separate Directories)
# ==========================================

# Pattern rule for Target 1 object files
$(BUILD_DIR1)/%.o: $(SRC_DIR1)/%.c
	@mkdir -p $(BUILD_DIR1)
	@echo "Compiling Lexer: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Pattern rule for Target 2 object files
$(BUILD_DIR2)/%.o: $(SRC_DIR2)/%.c
	@mkdir -p $(BUILD_DIR2)
	@echo "Compiling VM: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# ==========================================
# Clean Rule
# ==========================================
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(BUILD_DIR1) $(BUILD_DIR2)
	rm -f $(TARGET1) $(TARGET2)
