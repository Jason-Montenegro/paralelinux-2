CXX = g++
CXXFLAGS = -g -Wall -pthread -std=c++23

BIN_DIR = bin

all: client server

client:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) src/Lego_Client.cpp src/Client_main.cpp src/SSLSocket.cc src/VSocket.cc src/Socket.cc \
	-o $(BIN_DIR)/lego_client -lssl -lcrypto

server:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I../filesystem-repl/src src/Lego_Server.cpp src/SSLSocket.cc src/VSocket.cc src/Socket.cc src/Server_main.cpp \
	../filesystem-repl/src/BitmapManager.cpp ../filesystem-repl/src/DiskDevice.cpp ../filesystem-repl/src/FileSystem.cpp \
	../filesystem-repl/src/InodeManager.cpp ../filesystem-repl/src/Repl.cpp src/Logger.cpp \
	-o $(BIN_DIR)/lego_server -lpthread -lssl -lcrypto

runServer: server
	./$(BIN_DIR)/lego_server

runClient: client
	./$(BIN_DIR)/lego_client

clean:
	rm -rf $(BIN_DIR)