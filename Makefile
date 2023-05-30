CC := gcc
CFLAGS := -Wall -Wpedantic -std=c99
BUILD_DIR := build

OBJ := $(BUILD_DIR)/ccsv.o
TEST := $(BUILD_DIR)/test
HED := ccsv.h

all: $(OBJ) $(TEST)

$(OBJ): ccsv.c $(HED)
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(TEST): $(OBJ) test.c $(HED)
	$(CC) $^ -o $@ $(CFLAGS)

run:
	./$(TEST)

.PHONY: all run
