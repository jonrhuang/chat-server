#pragma once

#include <atomic>
#include <string>
#include <thread>

class Client {
public:
    Client(const std::string& host = "127.0.0.1", int port = 8080);
    ~Client();

    void run();
    void stop();
    bool isConnected() const;

private:
    std::string host_; 
    int port_;
    std::atomic<bool> connected_;
    int socket_fd_;
    std::thread recv_thread_;
    std::string username;

    void connectToServer();
    void disconnectFromServer();
    void sendMessage(const std::string& message);
    void receiveLoop();
};