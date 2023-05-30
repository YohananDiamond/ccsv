CC := gcc
CFLAGS := -Wall -Wpedantic -std=c99
BUILD_DIR := build

OBJ := $(BUILD_DIR)/ccsv.o
EXE := $(BUILD_DIR)/ccsv

all: $(OBJ) $(EXE)

$(OBJ):
	mkdir -p $(BUILD_DIR)
	$(CC) -c ccsv.c -o $@ $(CFLAGS)

$(EXE): $(OBJ)
	$(CC) $(BUILD_DIR)/ccsv.o ccsv_shell.c -o $@ $(CFLAGS)

run: $(EXE)
	./run-tests.sh

.PHONY: all run
