CXX = g++
CXXFLAGS = -std=c++17 -Wall -pthread
CXXFLAGS += $(shell pkg-config --cflags libmongocxx libmongoc-1.0)

SERVER_DIR = Server/
CLIENT_DIR = Client/

SERVER_LIBS = $(shell pkg-config --libs libmongocxx libmongoc-1.0)

SERVER_SOURCES = $(SERVER_DIR)main.cpp $(SERVER_DIR)server.cpp
CLIENT_SOURCES = $(CLIENT_DIR)main.cpp $(CLIENT_DIR)client.cpp

all: server client

server: $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(SERVER_LIBS)

client: $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f server client

.PHONY: clean run-server run-client

run-server: server
	./server

run-client: client
	./client