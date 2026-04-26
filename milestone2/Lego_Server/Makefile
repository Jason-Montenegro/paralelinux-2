CXX = g++
CXXFLAGS = -std=c++23

BIN_DIR = bin

all: client server

client:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) Lego_Client.cpp Client_main.cpp SSLSocket.cc VSocket.cc Socket.cc \
	-o $(BIN_DIR)/lego_client -lssl -lcrypto

server:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -I../filesystem-repl/src Lego_Server.cpp SSLSocket.cc VSocket.cc Socket.cc Server_main.cpp \
	../filesystem-repl/src/BitmapManager.cpp ../filesystem-repl/src/DiskDevice.cpp ../filesystem-repl/src/FileSystem.cpp \
	../filesystem-repl/src/InodeManager.cpp ../filesystem-repl/src/Repl.cpp Logger.cpp \
	-o $(BIN_DIR)/lego_server -lpthread -lssl -lcrypto

runServer: server
	./$(BIN_DIR)/lego_server

runClient: client
	./$(BIN_DIR)/lego_client

clean:
	rm -rf $(BIN_DIR)