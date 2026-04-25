CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -pthread -g

SRCS = Main.cpp \
       Message.cpp \
       MessageType.cc \
       Client.cpp \
       ClientConsoleHandler.cpp \
       Intermediary.cpp \
       RouteTable.cpp \
       Server.cpp \
       LegoManager.cpp \
       LegoRepository.cpp \
       Logger.cpp

# Carpeta donde se guardarán los .o y el ejecutable
BIN_DIR  = bin
OBJS_DIR = $(BIN_DIR)/obj

# Generar lista de objetos con la ruta de la carpeta bin/obj
OBJS = $(addprefix $(OBJS_DIR)/, $(notdir $(SRCS:.cpp=.o)))
OBJS := $(OBJS:.cc=.o)

BIN  = $(BIN_DIR)/lego_sim

# Crear carpetas necesarias
$(shell mkdir -p $(OBJS_DIR) $(BIN_DIR))

all: $(BIN)

$(BIN): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJS_DIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(OBJS_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<

run: $(BIN)
	./$(BIN)

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean run