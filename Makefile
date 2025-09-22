CC=gcc
CFLAGS=-Wall -Wextra -Iinclude
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)
TARGET=main

TEST_SRC=$(wildcard tests/*.c)
TEST_OBJ=$(TEST_SRC:.c=.o)
TEST_TARGET=test

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o build/$@ $^

$(TEST_TARGET): $(OBJ) $(TEST_OBJ)
	$(CC) $(CFLAGS) -o build/$@ $^

test: $(TEST_TARGET)
	./build/$(TEST_TARGET)

clean:
	rm -f src/*.o tests/*.o build/$(TARGET) build/$(TEST_TARGET)

.PHONY: all clean test
