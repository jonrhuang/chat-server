CXX = g++
CXXFLAGS = -std=c++11 -Wall -pthread

SERVER_DIR = Server/
CLIENT_DIR = Client/

SERVER_SOURCES = $(SERVER_DIR)main.cpp $(SERVER_DIR)server.cpp
CLIENT_SOURCES = $(CLIENT_DIR)main.cpp $(CLIENT_DIR)client.cpp

all: server client

server: $(SERVER_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

client: $(CLIENT_SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f server client

.PHONY: clean run-server run-client

run-server: server
	./server

run-client: client
	./client