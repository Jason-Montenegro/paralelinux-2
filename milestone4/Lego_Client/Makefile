CXX = g++
CXXFLAGS = -std=c++23

BIN_DIR = bin
SRC_DIR = src

UNAME_S := $(shell uname -s)

ifeq ($(UNAME_S),Darwin)
	CXXFLAGS += -I/opt/homebrew/opt/openssl@3.6/include -L/opt/homebrew/opt/openssl@3.6/lib
endif

.PHONY: run clean

all: client 

client:
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) \
	$(SRC_DIR)/Lego_Client.cpp \
	$(SRC_DIR)/Client_main.cpp \
	$(SRC_DIR)/SSLSocket.cc \
	$(SRC_DIR)/VSocket.cc \
	$(SRC_DIR)/Socket.cc \
	-o $(BIN_DIR)/lego_client -lssl -lcrypto

run:
	$(BIN_DIR)/lego_client

clean:
	rm -rf $(BIN_DIR)

