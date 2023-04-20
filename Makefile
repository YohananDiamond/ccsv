CC := gcc
CFLAGS := -Wall -Wpedantic -std=c99
BUILD_DIR := build

all: library executable

library:
	mkdir -p $(BUILD_DIR)
	$(CC) -c ccsv.c -o $(BUILD_DIR)/ccsv.o $(CFLAGS)

executable: library
	$(CC) $(BUILD_DIR)/ccsv.o ccsv_shell.c -o $(BUILD_DIR)/ccsv $(CFLAGS)

run: executable
	./run-tests.sh

.PHONY: all library executable run
