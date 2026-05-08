# ===================== Compiler & Flags =====================
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE -Iinclude \
           $(shell pkg-config --cflags gtk+-3.0 gdk-pixbuf-2.0) \
           -O3 -march=native -flto -fsanitize=address -g

LDFLAGS  = -lm \
           $(shell pkg-config --libs gtk+-3.0 gdk-pixbuf-2.0) \
           -fsanitize=address -g

# ===================== Directories =====================
SRC_DIR    = src
IMG_DIR    = $(SRC_DIR)/image
TEST_DIR   = tests
BUILD_DIR  = build

# ===================== Targets =====================
TARGET       = $(BUILD_DIR)/main
IMAGE_BIN    = $(BUILD_DIR)/image
PIPELINE_BIN = $(BUILD_DIR)/pipeline
NEURAL_BIN	 = $(BUILD_DIR)/neural

# ===================== Sources =====================
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)

# Exclude image/main.c
IMG_FILES = $(filter-out $(IMG_DIR)/main.c, $(wildcard $(IMG_DIR)/*.c))

# Remove unwanted files from main build
MAIN_SRC  = $(filter-out \
    $(SRC_DIR)/pipeline_main.c \
    $(SRC_DIR)/neural_network_main.c, \
    $(SRC_FILES))

# ===================== Objects =====================
MAIN_OBJ   = $(MAIN_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
IMG_OBJ    = $(IMG_FILES:$(IMG_DIR)/%.c=$(BUILD_DIR)/image_%.o)

SOLVER_OBJ = $(BUILD_DIR)/solver.o
NEURAL_OBJ = $(BUILD_DIR)/neural_network.o
GRID_OBJ   = $(BUILD_DIR)/grid_cutting.o
TIMER_OBJ  = $(BUILD_DIR)/timer.o

COMMON_OBJ = $(SOLVER_OBJ) $(NEURAL_OBJ) $(GRID_OBJ) $(IMG_OBJ) $(TIMER_OBJ)

PIPELINE_MAIN = $(BUILD_DIR)/pipeline_main.o
NEURAL_MAIN   = $(BUILD_DIR)/neural_network_main.o

# ===================== Build Rules =====================
.PHONY: all run pipeline neural clean tests

all: run

run: $(TARGET)
	@echo "Starting app..."
	@LSAN_OPTIONS=suppressions=lsan.supp ./$(TARGET)

pipeline: $(PIPELINE_BIN)
	@echo "Starting pipeline..."

neural: $(NEURAL_BIN)
	@echo "Starting neural network..."

# ---------- Main ----------
$(TARGET): $(MAIN_OBJ) $(COMMON_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking Main..."
	@$(CC) -o $@ $^ $(LDFLAGS)

# ---------- Image tool ----------
$(IMAGE_BIN): $(IMG_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking Image tool..."
	@$(CC) -o $@ $^ $(LDFLAGS)

# ---------- Pipeline ----------
$(PIPELINE_BIN): $(PIPELINE_MAIN) $(COMMON_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking Pipeline..."
	@$(CC) -o $@ $^ $(LDFLAGS)

# ---------- Neural network ----------
$(NEURAL_BIN): $(NEURAL_MAIN) $(COMMON_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking Neural..."
	@$(CC) -o $@ $^ $(LDFLAGS)

# ===================== Compilation =====================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/image_%.o: $(IMG_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# ===================== Tests =====================
TEST_FILES = $(filter-out $(TEST_DIR)/test_helpers.c, $(wildcard $(TEST_DIR)/*.c))
TEST_BINS  = $(TEST_FILES:$(TEST_DIR)/%.c=$(BUILD_DIR)/test_%)
TEST_HELPERS_OBJ = $(BUILD_DIR)/test_helpers.o

tests: $(TEST_BINS)
	@echo "Running tests..."
	@for t in $(TEST_BINS); do ./$$t; done

# Compile helpers separately
$(TEST_HELPERS_OBJ): $(TEST_DIR)/test_helpers.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling test_helpers.c..."
	@$(CC) $(CFLAGS) -DTESTING -c $< -o $@

# Build each test
$(BUILD_DIR)/test_%: $(TEST_DIR)/%.c $(COMMON_OBJ) $(TEST_HELPERS_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Building test $@..."
	@$(CC) $(CFLAGS) -DTESTING \
	    $< \
	    $(COMMON_OBJ) \
	    $(TEST_HELPERS_OBJ) \
	    -o $@ \
	    $(LDFLAGS)

# ===================== Clean =====================
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@rm -rf tests/results
	@echo "Clean complete."