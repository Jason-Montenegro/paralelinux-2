CXX = g++
CXXFLAGS = -std=c++23

BIN_DIR = bin
SRC_DIR = src

all: client server inter

client:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) \
	$(SRC_DIR)/Lego_Client.cpp \
	$(SRC_DIR)/Client_main.cpp \
	$(SRC_DIR)/SSLSocket.cc \
	$(SRC_DIR)/VSocket.cc \
	$(SRC_DIR)/Socket.cc \
	-o $(BIN_DIR)/lego_client -lssl -lcrypto

server:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) \
	-I../filesystem-repl/src \
	$(SRC_DIR)/Lego_Server.cpp \
	$(SRC_DIR)/Server_main.cpp \
	$(SRC_DIR)/SSLSocket.cc \
	$(SRC_DIR)/VSocket.cc \
	$(SRC_DIR)/Socket.cc \
	$(SRC_DIR)/Logger.cpp \
	../filesystem-repl/src/BitmapManager.cpp \
	../filesystem-repl/src/DiskDevice.cpp \
	../filesystem-repl/src/FileSystem.cpp \
	../filesystem-repl/src/InodeManager.cpp \
	../filesystem-repl/src/Repl.cpp \
	-o $(BIN_DIR)/lego_server \
	-lpthread -lssl -lcrypto

inter:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) \
	-I../filesystem-repl/src \
	$(SRC_DIR)/Lego_Inter.cpp \
	$(SRC_DIR)/Inter_main.cpp \
	$(SRC_DIR)/Lego_Client.cpp \
	$(SRC_DIR)/SSLSocket.cc \
	$(SRC_DIR)/VSocket.cc \
	$(SRC_DIR)/Socket.cc \
	$(SRC_DIR)/LoggerFiguras.cpp \
	../filesystem-repl/src/BitmapManager.cpp \
	../filesystem-repl/src/DiskDevice.cpp \
	../filesystem-repl/src/FileSystem.cpp \
	../filesystem-repl/src/InodeManager.cpp \
	../filesystem-repl/src/Repl.cpp \
	-o $(BIN_DIR)/lego_inter \
	-lpthread -lssl -lcrypto

clean:
	rm -rf $(BIN_DIR)
