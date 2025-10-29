# ===================== Compiler & Flags =====================
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -Iinclude $(shell pkg-config --cflags gtk+-3.0 gdk-pixbuf-2.0)
LDFLAGS  = -lm $(shell pkg-config --libs gtk+-3.0 gdk-pixbuf-2.0)

# ===================== Directories =====================
SRC_DIR      = src
IMG_DIR      = $(SRC_DIR)/image
INCLUDE_DIR  = include
TEST_DIR     = tests
BUILD_DIR    = build
RESULTS_DIR  = $(TEST_DIR)/results

# ===================== Target Binaries =====================
TARGET       = $(BUILD_DIR)/main
SOLVER_BIN   = $(BUILD_DIR)/solver
UI_BIN       = $(BUILD_DIR)/ui
IMAGE_BIN    = $(BUILD_DIR)/image

# ===================== Source Files =====================
SRC_FILES    = $(wildcard $(SRC_DIR)/*.c)
IMG_FILES    = $(wildcard $(IMG_DIR)/*.c)
TEST_FILES   = $(wildcard $(TEST_DIR)/*.c)

MAIN_SRC     = $(filter-out $(SRC_DIR)/solver.c $(SRC_DIR)/ui.c, $(SRC_FILES))
IMG_UI_SRC   = $(filter-out $(IMG_DIR)/main.c, $(IMG_FILES))

# ===================== Object Files =====================
MAIN_OBJ     = $(MAIN_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
SOLVER_OBJ   = $(BUILD_DIR)/solver.o
UI_OBJ       = $(BUILD_DIR)/ui.o $(IMG_UI_SRC:$(IMG_DIR)/%.c=$(BUILD_DIR)/image_%.o)
IMG_OBJ      = $(IMG_FILES:$(IMG_DIR)/%.c=$(BUILD_DIR)/image_%.o)

# ===================== Main Rules =====================
all: $(TARGET) $(SOLVER_BIN) $(UI_BIN) $(IMAGE_BIN) tests

# ---------- Main Program ----------
$(TARGET): $(MAIN_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Main program built successfully."

# ---------- Solver ----------
$(SOLVER_BIN): $(SOLVER_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Solver built successfully."

# ---------- UI ----------
UI_SRC      = $(SRC_DIR)/ui.c

$(UI_BIN): $(UI_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "UI built successfully."

$(BUILD_DIR)/ui.o: $(SRC_DIR)/ui.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# ---------- Image Program ----------
$(IMAGE_BIN): $(IMG_OBJ)
	@mkdir -p $(BUILD_DIR)
	$(CC) -o $@ $^ $(LDFLAGS)
	@echo "Image program built successfully."

# ===================== Generic Compilation =====================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/image_%.o: $(IMG_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/neural-network.o: $(SRC_DIR)/neural-network.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -DTESTING -c $< -o $@

# ===================== Tests Compilation =====================
TEST_BINS = $(TEST_FILES:$(TEST_DIR)/%.c=$(BUILD_DIR)/test_%)

tests: $(TEST_BINS)
	@echo "All tests built successfully."
	@echo "Running tests..."
	@for t in $(TEST_BINS); do ./$$t; done

$(BUILD_DIR)/test_%: $(TEST_DIR)/%.c \
                     $(filter-out $(BUILD_DIR)/neural-network.o $(BUILD_DIR)/image_main.o,$(MAIN_OBJ)) \
                     $(IMG_UI_SRC:$(IMG_DIR)/%.c=$(BUILD_DIR)/image_%.o)
	@mkdir -p $(BUILD_DIR)
	@if echo $@ | grep -q "poc_neuronal"; then \
		$(MAKE) $(BUILD_DIR)/neural-network.o; \
		$(CC) $(CFLAGS) -DTESTING -o $@ $^ $(BUILD_DIR)/neural-network.o $(LDFLAGS); \
	else \
		$(CC) $(CFLAGS) -DTESTING -o $@ $^ $(LDFLAGS); \
	fi

# ===================== Clean =====================
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@rm -f $(TARGET) $(SOLVER_BIN) $(UI_BIN) $(IMAGE_BIN)
	@rm -rf $(RESULTS_DIR)/*
	@echo "Clean complete."

.PHONY: all clean tests
