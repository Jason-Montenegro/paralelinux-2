# Copyright (C) 2026 Jason Montenegro-Navarro <jasonmontenegro49@gmail.com>
SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin
BIN_NAME := ezfs 

CXX := g++
XSTD := -std=c++23
CXXFLAGS := $(XSTD)\
						-Wall\
						-Wextra

INCLUDE_DIRS := -I$(SRC_DIR)

SRCS := $(shell find $(SRC_DIR) -name "*.cpp")
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

ifeq ($(BUILD),debug)
	LDFLAGS := -fsanitize=address,undefined
	CXXFLAGS += -g -Og -DDEBUG -fsanitize=address,undefined
else
	CXXFLAGS += -O2
endif

.DELETE_ON_ERROR:
all: $(BIN_DIR)/$(BIN_NAME)

$(BIN_DIR)/$(BIN_NAME): $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE_DIRS) -c $< -o $@

run: $(BIN_DIR)/$(BIN_NAME)
	$(BIN_DIR)/$(BIN_NAME)

FORMAT_FILENAMES := $(shell find $(SRC_DIR) -name "*.cpp" -o -name "*.hpp")
format:
ifndef ($(shell command -v clang-format),)
	@clang-format -i $(FORMAT_FILENAMES)
else
	@echo "You must have clang-format tool, installed in your machine."
endif

lint:
ifndef ($(shell command -v clang-tidy),)
	@clang-tidy --config-file=.clang-tidy $(SRCS) -- $(XSTD) $(INCLUDE_DIRS)
else
	@echo "You must have clang-tidy tool, installed in your machine."
endif

clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

ESC := \033[
BOLD := $(ESC)1m
LIGHT_GREEN := $(ESC)92m
LIGHT_YELLOW := $(ESC)93m
RESET_ANSI := $(ESC)0m

help:
	@printf "$(BOLD)=== Target list ===$(RESET_ANSI)\n"
	@printf "$(LIGHT_GREEN)$(BOLD)make$(RESET_ANSI)               : compiles optimizing with -O2\n"
	@printf "$(LIGHT_GREEN)$(BOLD)make run$(RESET_ANSI)           : executes program\n"
	@printf "$(LIGHT_GREEN)$(BOLD)make BUILD=debug$(RESET_ANSI)   : compiles in debug mode\n"
	@printf "$(LIGHT_GREEN)$(BOLD)make format$(RESET_ANSI)        : formats source code (must have clang-format installed)\n"
	@printf "$(LIGHT_GREEN)$(BOLD)make lint$(RESET_ANSI)          : lints source code (must have clang-tidy installed)\n"
	@printf "$(LIGHT_GREEN)$(BOLD)make help$(RESET_ANSI)          : prints this target list\n"
	@printf "\n"
	@printf "$(LIGHT_YELLOW)$(BOLD)Runtime Arguments$(RESET_ANSI)  : set DISK_FILENAME (defaulted to "ez_container.bin")\n"

.PHONY: all clean run format lint
