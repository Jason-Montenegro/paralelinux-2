# Copyright 2026 Paralelinux-2

SRC_DIR=src
BUILD_DIR=build
BIN_DIR=bin
LOG_DIR=logs
LIBS_DIR=protocol
BIN_NAME=simulation
INCLUDES = -I$(SRC_DIR)/ -I$(LIBS_DIR)/

# Runtime environment variables for running the simulation.
# Modify if you have a different compiler. e.g. clang, MSVC, etc.
CXX=g++-15
CXX_FLAGS=-std=c++20 \
					-Wall \
					-Wextra \
					-Wpedantic \
          -Wnon-virtual-dtor \
          -Wold-style-cast \
          -Wcast-align \
          -Woverloaded-virtual \
          -Wconversion \
          -Wsign-conversion \
          -Wnull-dereference \
          -Wdouble-promotion

SRC=$(wildcard $(SRC_DIR)/*.cpp)
SRC_OBJS=$(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC))
LIB_SRCS=$(wildcard $(LIBS_DIR)/*.cpp)
LIB_OBJS=$(patsubst $(LIBS_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(LIB_SRCS))

all: $(BIN_DIR)/$(BIN_NAME)

run: all
	$(BIN_DIR)/$(BIN_NAME)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR)/$(BIN_NAME): $(SRC_OBJS) $(LIB_OBJS) | $(BIN_DIR)
	$(CXX) $(CXX_FLAGS) $(INCLUDES) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/%.o: $(LIBS_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXX_FLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)
	rm -rf $(BIN_DIR)

clean-logs:
	rm -rf $(LOG_DIR)

help:
	@echo "Available targets"
	@echo "\tmake       		 : compiles the program"
	@echo "\tmake run   		 : compiles and runs the program"
	@echo "\tmake clean 		 : erase all compiled files"
	@echo "\tmake clean-logs : erase logs/ directory"
	@echo "\tmake help  		 : get this target list"

.PHONY: all run clean help
