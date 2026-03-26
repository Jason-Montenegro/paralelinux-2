# Copyright 2026 Paralelinux-2

SRC_DIR=src
BUILD_DIR=build
BIN_DIR=bin
BIN_NAME=lego_client
LIBS = -lssl -lcrypto
INCLUDES = -I$(SRC_DIR)/


# Runtime environment variables for running the simulation.
# Modify if you have a different compiler. e.g. clang, MSVC, etc.
CXX=g++
CXX_FLAGS=-std=c++20 \
					-Wall \
					-Wextra \
	

SRC=$(wildcard $(SRC_DIR)/*.cc)
SRC_OBJS=$(patsubst $(SRC_DIR)/%.cc, $(BUILD_DIR)/%.o, $(SRC))

all: $(BIN_DIR)/$(BIN_NAME)

run: all
	$(BIN_DIR)/$(BIN_NAME)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)


$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc | $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

$(BIN_DIR)/$(BIN_NAME): $(SRC_OBJS) | $(BIN_DIR)
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $^ -o $@ $(LIBS)

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)

help:
	@echo "Available targets"
	@echo "	make           : compiles the program"
	@echo "	make run       : compiles and runs the program"
	@echo "	make clean     : erase all compiled files"
	@echo "	make build-ssl : build OpenSSL library"
	@echo "	make usage     : print the way to run the program with the make"
	@echo "	make help      : get this target list"

.PHONY: all run clean help