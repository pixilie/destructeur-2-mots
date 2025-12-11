# ===================== Compiler & Flags =====================
CC        = gcc
CFLAGS    = -Wall -Wextra -std=c99 -D_DEFAULT_SOURCE -Iinclude -Iinclude/image $(shell pkg-config --cflags gtk+-3.0 gdk-pixbuf-2.0) -fsanitize=address -g
LDFLAGS   = -lm $(shell pkg-config --libs gtk+-3.0 gdk-pixbuf-2.0) -fsanitize=address -g

# ===================== Directories =====================
SRC_DIR      = src
IMG_DIR      = $(SRC_DIR)/image
BUILD_DIR    = build
TEST_DIR     = tests

# ===================== Target Binaries =====================
TARGET       = $(BUILD_DIR)/main

# ===================== Source Files =====================
IMG_SRCS     = $(wildcard $(IMG_DIR)/*.c)
CORE_SRCS    = $(filter-out $(SRC_DIR)/main.c, $(wildcard $(SRC_DIR)/*.c))

# ===================== Object Files =====================
IMG_OBJS     = $(IMG_SRCS:$(IMG_DIR)/%.c=$(BUILD_DIR)/image_%.o)
CORE_OBJS    = $(CORE_SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
MAIN_OBJ     = $(BUILD_DIR)/main.o
ALL_OBJS     = $(CORE_OBJS) $(IMG_OBJS)

# ===================== Main Rules =====================
all: $(TARGET)
	@echo "Starting the application..."
	@./$(TARGET)

$(TARGET): $(MAIN_OBJ) $(ALL_OBJS)
	@mkdir -p $(BUILD_DIR)
	@echo "Linking $@"
	@$(CC) -o $@ $^ $(LDFLAGS)

# ===================== Compilation Rules =====================
$(BUILD_DIR)/main.o: $(SRC_DIR)/main.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/image_%.o: $(IMG_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

# ===================== Clean =====================
clean:
	@echo "Nettoyage des fichiers de build..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(TEST_DIR)/results
	@echo "Clean terminé."

.PHONY: all clean
