CC = gcc

CFLAGS = -Wall -Wextra -Iinclude `pkg-config --cflags gtk+-3.0`

LDFLAGS = `pkg-config --libs gtk+-3.0`

SRC = $(wildcard src/*.c)
OBJ = $(SRC:.c=.o)

TARGET = main
IMAGE_TARGET = image
TEST_TARGET = test

TEST_SRC = $(wildcard tests/*.c)
TEST_OBJ = $(TEST_SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o build/$@ $^ $(LDFLAGS)

$(IMAGE_TARGET):
	$(CC) $(CFLAGS) `pkg-config --cflags gtk+-3.0` -o build/$@ src/image.c `pkg-config --libs gtk+-3.0`

test: $(TEST_TARGET)
	./build/$(TEST_TARGET)

clean:
	rm -f src/*.o tests/*.o build/$(TARGET) build/$(TEST_TARGET) build/$(IMAGE_TARGET)

.PHONY: all clean test
