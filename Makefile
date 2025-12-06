# ===================== Compiler & Flags =====================
CC       = gcc
CFLAGS   = -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE -Iinclude $(shell pkg-config --cflags gtk+-3.0 gdk-pixbuf-2.0) -fsanitize=address -g
LDFLAGS  = -lm $(shell pkg-config --libs gtk+-3.0 gdk-pixbuf-2.0) -fsanitize=address -g

# ===================== Directories =====================
SRC_DIR      = src
IMG_DIR      = $(SRC_DIR)/image
UI_SRC_DIR   = $(SRC_DIR)/ui
INCLUDE_DIR  = include
TEST_DIR     = tests
BUILD_DIR    = build
RESULTS_DIR  = $(TEST_DIR)/results

# ===================== Target Binaries =====================
TARGET       = $(BUILD_DIR)/main
UI_BIN       = $(BUILD_DIR)/ui
IMAGE_BIN    = $(BUILD_DIR)/image
PIPELINE_BIN = $(BUILD_DIR)/pipeline

# ===================== Source Files =====================
SRC_FILES    = $(wildcard $(SRC_DIR)/*.c)
IMG_FILES    = $(wildcard $(IMG_DIR)/*.c)
UI_FILES     = $(wildcard $(UI_SRC_DIR)/*.c)
TEST_FILES   = $(filter-out $(TEST_DIR)/test_helpers.c, $(wildcard $(TEST_DIR)/*.c))

MAIN_SRC = $(filter-out \
    $(SRC_DIR)/solver.c \
    $(SRC_DIR)/neural_network.c \
    $(SRC_DIR)/ui.c \
    $(SRC_DIR)/line_detection.c, \
    $(SRC_FILES))

IMG_UI_SRC   = $(filter-out $(IMG_DIR)/main.c,$(IMG_FILES))
IMG_PIPE_SRC = $(filter-out $(IMG_DIR)/main.c,$(IMG_FILES))

# ===================== Object Files =====================
MAIN_OBJ          = $(MAIN_SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
CORE_OBJ = $(filter-out $(BUILD_DIR)/image_main.o, \
              $(IMG_FILES:$(IMG_DIR)/%.c=$(BUILD_DIR)/image_%.o)) \
            $(BUILD_DIR)/dataset.o \
            $(BUILD_DIR)/solver.o \
            $(BUILD_DIR)/neural_network.o
UI_OBJ = $(CORE_OBJ) build/ui.o build/ui_solve_grid.o
IMG_OBJ           = $(IMG_FILES:$(IMG_DIR)/%.c=$(BUILD_DIR)/image_%.o)
PIPELINE_OBJ      = $(BUILD_DIR)/line_detection.o
PIPELINE_IMG_OBJ  = $(IMG_PIPE_SRC:$(IMG_DIR)/%.c=$(BUILD_DIR)/image_%.o)
SOLVER_OBJ        = $(BUILD_DIR)/solver.o
NEURAL_NET_OBJ    = $(BUILD_DIR)/neural_network.o

# ===================== Main Rules =====================
all: $(TARGET) $(UI_BIN) $(IMAGE_BIN) $(PIPELINE_BIN)

# ---------- Main Program ----------
$(TARGET): $(MAIN_OBJ) $(SOLVER_OBJ) $(NEURAL_NET_OBJ) $(filter-out $(BUILD_DIR)/image_main.o, $(IMG_OBJ))
	@mkdir -p $(BUILD_DIR)
	@echo "Linking Main..."
	@$(CC) -o $@ $^ $(LDFLAGS)

# ---------- UI ----------
$(UI_BIN): $(UI_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking UI..."
	@$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(UI_SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# ---------- Image Program ----------
$(IMAGE_BIN): $(IMG_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking Image tool..."
	@$(CC) -o $@ $^ $(LDFLAGS)

# ---------- Pipeline Program ----------
$(PIPELINE_BIN): $(PIPELINE_OBJ) $(PIPELINE_IMG_OBJ) $(SOLVER_OBJ) $(NEURAL_NET_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking Pipeline..."
	@$(CC) -o $@ $(PIPELINE_OBJ) $(PIPELINE_IMG_OBJ) $(SOLVER_OBJ) $(NEURAL_NET_OBJ) $(LDFLAGS)

$(BUILD_DIR)/line_detection.o: $(SRC_DIR)/line_detection.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# ===================== Generic Compilation =====================
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/image_%.o: $(IMG_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(NN_OBJ): $(SRC_DIR)/neural_network.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(SOLVER_OBJ): $(SRC_DIR)/solver.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $(SRC_DIR)/solver.c -o $(SOLVER_OBJ)

# ===================== Tests Compilation =====================
TEST_OBJ = $(filter-out $(BUILD_DIR)/main.o,$(MAIN_OBJ)) $(SOLVER_OBJ) $(NEURAL_NET_OBJ)
TEST_BINS = $(TEST_FILES:$(TEST_DIR)/%.c=$(BUILD_DIR)/test_%)
LINE_DET_TEST      = $(BUILD_DIR)/test_line_detection_tests
LINE_DET_TEST_SRC  = $(TEST_DIR)/line_detection_tests.c
LINE_DET_HELPERS   = $(TEST_DIR)/test_helpers.c
LINE_DET_OBJ       = $(BUILD_DIR)/test_line_detection.o

tests: $(TEST_BINS)
	@echo "All tests built successfully."
	@echo "Running tests..."
	@for t in $(TEST_BINS); do ./$$t; done

$(BUILD_DIR)/test_helpers.o: $(TEST_DIR)/test_helpers.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling test_helpers.c..."
	@$(CC) $(CFLAGS) -DTESTING -c $< -o $@


$(BUILD_DIR)/test_%: $(TEST_DIR)/%.c $(BUILD_DIR)/test_helpers.o $(TEST_OBJ) $(PIPELINE_IMG_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Building test: $@"
	@$(CC) $(CFLAGS) -DTESTING -c $< -o $(BUILD_DIR)/$*_test.o
	@$(CC) $(CFLAGS) -DTESTING -c $(SRC_DIR)/line_detection.c -o $(BUILD_DIR)/line_detection.o
	@$(CC) -o $@ $(BUILD_DIR)/$*_test.o $(BUILD_DIR)/line_detection.o $(BUILD_DIR)/test_helpers.o $(TEST_OBJ) $(PIPELINE_IMG_OBJ) $(LDFLAGS)

$(BUILD_DIR)/test_solver_tests: $(TEST_DIR)/solver_tests.c $(TEST_DIR)/test_helpers.c \
                                $(SOLVER_OBJ) $(NEURAL_NET_OBJ) $(PIPELINE_OBJ) $(PIPELINE_IMG_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Building test: $@"
	@$(CC) $(CFLAGS) -DTESTING -c $(TEST_DIR)/test_helpers.c -o $(BUILD_DIR)/test_helpers.o
	@$(CC) $(CFLAGS) -DTESTING -c $(TEST_DIR)/solver_tests.c -o $(BUILD_DIR)/solver_tests.o
	@$(CC) $(CFLAGS) -DTESTING -o $@ \
	    $(BUILD_DIR)/solver_tests.o \
	    $(BUILD_DIR)/test_helpers.o \
	    $(SOLVER_OBJ) \
	    $(NEURAL_NET_OBJ) \
	    $(PIPELINE_OBJ) \
	    $(PIPELINE_IMG_OBJ) \
	    $(LDFLAGS)

$(BUILD_DIR)/test_line_detection_tests: $(LINE_DET_TEST_SRC) $(LINE_DET_HELPERS) $(PIPELINE_IMG_OBJ) $(SOLVER_OBJ) $(NEURAL_NET_OBJ)
	@mkdir -p $(BUILD_DIR)
	@echo "Building test: $@"
	@$(CC) $(CFLAGS) -DTESTING -c $(LINE_DET_HELPERS) -o $(BUILD_DIR)/test_helpers.o
	@$(CC) $(CFLAGS) -DTESTING -c $(LINE_DET_TEST_SRC) -o $(BUILD_DIR)/test_line_detection_tests.o
	@$(CC) $(CFLAGS) -DTESTING -c $(SRC_DIR)/line_detection.c -o $(BUILD_DIR)/line_detection.o
	@$(CC) $(CFLAGS) -DTESTING -o $@ \
	    $(BUILD_DIR)/test_line_detection_tests.o \
	    $(BUILD_DIR)/test_helpers.o \
	    $(BUILD_DIR)/line_detection.o \
	    $(SOLVER_OBJ) \
	    $(NEURAL_NET_OBJ) \
	    $(PIPELINE_IMG_OBJ) \
	    $(LDFLAGS)

# ===================== Clean =====================
clean:
	@echo "Cleaning build files..."
	@find $(BUILD_DIR) -mindepth 1 ! -name ".__afs*" -exec rm -rf {} + 2>/dev/null || true
	@rm -rf $(RESULTS_DIR) 2>/dev/null || true
	#@rm -f tests/model
	@rm -rf solver_output/
	@rm -f image.png bw.png filtered.png
	@rm -rf gw/ letters/
	@rm -rf helpers/generate_dataset/fonts helpers/generate_dataset/dataset
	@echo "Clean complete."

.PHONY: all clean tests
