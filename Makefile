CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lm

BUILD_DIR = build
TARGET = main
SOLVER_TARGET = solver
TEST_TARGET = test

SRC = $(filter-out src/solver.c, $(wildcard src/*.c))
OBJ = $(SRC:.c=.o)

TEST_SRC = $(wildcard tests/*.c)
TEST_OBJ = $(TEST_SRC:.c=.o)

all: $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(SOLVER_TARGET) $(BUILD_DIR)/$(TEST_TARGET)

solver: $(BUILD_DIR)/$(SOLVER_TARGET)
	@echo "Solver built successfully."

tests: $(BUILD_DIR)/$(TEST_TARGET)
	@echo "Running tests..."
	@./$(BUILD_DIR)/$(TEST_TARGET)
	@echo "Tests completed."

clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR) src/*.o tests/*.o
	@echo "Clean complete."

$(BUILD_DIR)/$(TARGET): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
	@echo "Main program built successfully."

$(BUILD_DIR)/$(SOLVER_TARGET): src/solver.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(BUILD_DIR)/$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: all solver tests clean
