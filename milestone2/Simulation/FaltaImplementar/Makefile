CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pthread -g

SRCS = main.cpp \
    Client1.cpp \
    ClientConsoleHandler.cpp \
    Intermediary1.cpp \
    Switch.cpp \
    Simulation.cpp \
    Server1.cpp \
	LegoManager1.cpp \
    ../RouteTable.cpp \
    ../LegoRepository.cpp \
    ../Logger.cpp

BIN_DIR  = bin
OBJS_DIR = $(BIN_DIR)/obj

OBJS = $(patsubst %.cpp,$(OBJS_DIR)/%.o,$(SRCS))

BIN = $(BIN_DIR)/switch

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJS_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean run