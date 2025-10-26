CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lm

SRC = $(wildcard src/.c)
OBJ = $(SRC:.c=.o)
TARGET = main
BUILD_DIR = build

TEST_SRC = $(wildcard tests/.c)
TEST_OBJ = $(TEST_SRC:.c=.o)
TEST_TARGET = test

all: run

$(BUILD_DIR)/$(TARGET): $(OBJ)
    @mkdir -p $(BUILD_DIR)
    $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

run: $(BUILD_DIR)/$(TARGET)
    @echo "Running $(TARGET)..."
    @./$(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
    @mkdir -p $(BUILD_DIR)
    $(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

test: $(BUILD_DIR)/$(TEST_TARGET)
    @echo "Running tests..."
    @./$(BUILD_DIR)/$(TEST_TARGET)
    @echo "Tests ran."

clean:
    @echo "Cleaning build files..."
    @rm -f src/.o tests/.o $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(TEST_TARGET)
    @echo "Build files cleaned."

.PHONY: all clean test run
