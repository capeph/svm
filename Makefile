
# ==========================================
# Directory Definitions
# ==========================================
SRC_COMMON_DIR := common_src

APP1			:= lex
APP2			:= parser
APP3			:= vm

SRC1_DIR       := $(APP1)_src
SRC2_DIR       := $(APP2)_src
SRC3_DIR       := $(APP3)_src

BUILD1_DIR     := $(APP1)_build
BUILD2_DIR     := $(APP2)_build
BUILD3_DIR     := $(APP3)_build


# ==========================================
# Compiler and Flags Configuration
# ==========================================
CC       := gcc
CFLAGS   := -Wall -Wextra -O2 -I$(SRC_COMMON_DIR)
LDFLAGS  :=


# ==========================================
# Source File Discovery
# ==========================================
# Common sources used by every target
SRCS_COMMON    := $(wildcard $(SRC_COMMON_DIR)/*.c)

# Unique sources per target
SRCS1          := $(wildcard $(SRC1_DIR)/*.c)
SRCS2          := $(wildcard $(SRC2_DIR)/*.c)
SRCS3          := $(wildcard $(SRC3_DIR)/*.c)

# Aggregated sources per target (Target sources + Common sources)
ALL_SRCS1      := $(SRCS1) $(SRCS_COMMON)
ALL_SRCS2      := $(SRCS2) $(SRCS_COMMON)
ALL_SRCS3      := $(SRCS3) $(SRCS_COMMON)

# ==========================================
# Object File Mapping
# ==========================================
# Standardizes paths to completely isolate .o files inside target build directories
OBJS1          := $(patsubst %.c, $(BUILD1_DIR)/%.o, $(ALL_SRCS1))
OBJS2          := $(patsubst %.c, $(BUILD2_DIR)/%.o, $(ALL_SRCS2))
OBJS3          := $(patsubst %.c, $(BUILD3_DIR)/%.o, $(ALL_SRCS3))

# Final application binary names
TARGET1        := $(APP1)
TARGET2        := $(APP2)
TARGET3        := $(APP3)

# ==========================================
# Primary Rules & Phony Targets
# ==========================================
.PHONY: all clean target1 target2 target3

# Default rule builds everything
all: lex target2 target3

# Friendly aliases to build specific targets individually
target1: $(TARGET1)
target2: $(TARGET2)
target3: $(TARGET3)

# ==========================================
# Linking Rules (Executable generation)
# ==========================================
$(TARGET1): $(OBJS1)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS1) -o $@ $(LDFLAGS)

$(TARGET2): $(OBJS2)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS2) -o $@ $(LDFLAGS)

$(TARGET3): $(OBJS3)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS3) -o $@ $(LDFLAGS)

# ==========================================
# Compilation Rules (Target 1)
# ==========================================
$(BUILD1_DIR)/$(SRC1_DIR)/%.o: $(SRC1_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC1_DIR) -c $< -o $@

$(BUILD1_DIR)/$(SRC_COMMON_DIR)/%.o: $(SRC_COMMON_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ==========================================
# Compilation Rules (Target 2)
# ==========================================
$(BUILD2_DIR)/$(SRC2_DIR)/%.o: $(SRC2_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC2_DIR) -c $< -o $@

$(BUILD2_DIR)/$(SRC_COMMON_DIR)/%.o: $(SRC_COMMON_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ==========================================
# Compilation Rules (Target 3)
# ==========================================
$(BUILD3_DIR)/$(SRC3_DIR)/%.o: $(SRC3_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(SRC3_DIR) -c $< -o $@

$(BUILD3_DIR)/$(SRC_COMMON_DIR)/%.o: $(SRC_COMMON_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# ==========================================
# Clean Rule
# ==========================================
clean:
	rm -rf $(BUILD1_DIR) $(BUILD2_DIR) $(BUILD3_DIR) $(APP1) $(APP2) $(APP3)
