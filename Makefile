CC := gcc
CFLAGS := -Wall -Wpedantic -std=c99
BUILD_DIR := build

OBJ := $(BUILD_DIR)/ccsv.o
EXE := $(BUILD_DIR)/ccsv
HED := ccsv.h

all: $(OBJ) $(EXE)

$(OBJ): ccsv.c $(HED)
	mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

$(EXE): $(OBJ) ccsv_shell.c $(HED)
	$(CC) $^ -o $@ $(CFLAGS)

run: $(EXE)
	./run-tests.sh

.PHONY: all run
