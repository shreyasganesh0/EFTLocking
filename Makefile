CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
DEBUG_FLAGS = -g -DDEBUG
SRC_DIR = src
BUILD_DIR = build
TARGET_ETF = $(BUILD_DIR)/etf
TARGET_ETF_DEBUG = $(BUILD_DIR)/etf_debug

# Default target
all: $(TARGET_ETF) $(TARGET_ETF_DEBUG)

# Rule for building combiner binary directly
$(TARGET_ETF): $(SRC_DIR)/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

$(TARGET_ETF_DEBUG): $(SRC_DIR)/main_debug.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

# Debug mode target
debug: CFLAGS += $(DEBUG_FLAGS)
debug: clean all

# Clean up build files
clean:
	rm -rf $(BUILD_DIR)

# Phony targets
.PHONY: all clean debug

