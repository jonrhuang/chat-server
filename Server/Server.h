#ifndef SERVER_H
#define SERVER_H

class Server {
public:
    explicit Server(int port = 8080);
    ~Server();

    void run();
    void stop();
    bool isRunning() const;

private:
    int port_;
    bool running_;
    int listen_fd_;

    void init();
    void cleanup();
    bool createSocket();
    void acceptLoop();
    void handleClient(int clientSocket);
};

#endif