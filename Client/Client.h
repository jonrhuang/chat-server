#ifndef CLIENT_H
#define CLIENT_H

#include <string>

class Client {
public:
    Client(const std::string& host = "127.0.0.1", int port = 8080);
    ~Client();

    void run(std::string message = "Hello from Client");
    void stop();
    bool isConnected() const;

private:
    std::string host_; 
    int port_;
    bool connected_;
    int socket_fd_;

    void connectToServer();
    void disconnectFromServer();
    void sendMessage(const std::string& message);
    std::string receiveMessage();
};

#endif
