CXX = g++

CXXFLAGS = -std=c++20 -Wall -Wextra -pthread -g

SRCS = main.cpp \
       Server.cpp \
       Intermediary.cpp \
       Client.cpp \
       Logger.cpp \
       LegoRepository.cpp \
       ServerMap.cpp

BIN_DIR  = bin
OBJS_DIR = $(BIN_DIR)/obj

OBJS = $(addprefix $(OBJS_DIR)/, $(notdir $(SRCS:.cpp=.o)))

BIN = $(BIN_DIR)/simulador

$(shell mkdir -p $(OBJS_DIR) $(BIN_DIR))

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJS_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean run