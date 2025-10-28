CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
LDFLAGS = -lm
GTK_CFLAGS = $(shell pkg-config --cflags gtk+-3.0 gdk-pixbuf-2.0)
GTK_LIBS = $(shell pkg-config --libs gtk+-3.0 gdk-pixbuf-2.0)

BUILD_DIR = build
TARGET = main
SOLVER_TARGET = solver
IMAGE_TARGET = image
TEST_TARGET = test_exec

SRC = $(filter-out src/solver.c, $(wildcard src/*.c))
OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.o)

TEST_SRC = $(wildcard tests/*.c)
TEST_OBJ = $(TEST_SRC:tests/%.c=$(BUILD_DIR)/%.o)
TEST_SRC_OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.test.o) 

all: $(BUILD_DIR)/$(TARGET) $(BUILD_DIR)/$(SOLVER_TARGET) $(BUILD_DIR)/$(TEST_TARGET)

# ===================== Program builds =====================

$(BUILD_DIR)/$(TARGET): $(OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -o $@ $^ $(LDFLAGS) $(GTK_LIBS)
	@echo "Main program built successfully."

$(BUILD_DIR)/$(SOLVER_TARGET): src/solver.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)
	@echo "Solver built successfully."

$(BUILD_DIR)/$(IMAGE_TARGET): src/image.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -o $@ $^ $(LDFLAGS) $(GTK_LIBS)
	@echo "Image built successfully."

# ===================== Test builds =====================

tests: $(TEST_OBJ)
	@echo "Running tests..."
	@for t in $(TEST_OBJ); do \
		$(CC) $(CFLAGS) $(GTK_CFLAGS) -o $(BUILD_DIR)/$$(basename $$t .o) $$t $(TEST_SRC_OBJ) $(LDFLAGS) $(GTK_LIBS); \
		./$(BUILD_DIR)/$$(basename $$t .o); \
	done
	@echo "Tests completed."

$(BUILD_DIR)/$(TEST_TARGET): $(TEST_OBJ) $(TEST_SRC_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -o $@ $^ $(LDFLAGS) $(GTK_LIBS)

# ===================== Object compilation =====================

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.test.o: src/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -DTESTING -c $< -o $@

$(BUILD_DIR)/%.o: tests/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) $(GTK_CFLAGS) -DTESTING -c $< -o $@

# ===================== Clean =====================

clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR) src/*.o tests/*.o
	@echo "Clean complete."

.PHONY: all solver tests image clean

