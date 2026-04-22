#include "Client.h"

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

Client::Client(const std::string& host, int port)
    : host_(host), port_(port), connected_(false), socket_fd_(-1) {
}

Client::~Client() {
    stop();
}

void Client::connectToServer() {
  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ < 0) {
    std::cerr << "Socket creating failed: " << std::endl;
    return;
  }

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port_);
  if (inet_pton(AF_INET, host_.c_str(), &serverAddr.sin_addr) <= 0) {
    std::error_code ec(errno, std::generic_category());
    std::cerr << "Invalid IP: " << ec.message() << std::endl;
    close(socket_fd_);
    socket_fd_ = -1;
    return;
  }

  if (connect(socket_fd_, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) < 0) {
    std::cerr << "Could not connect to server: " << std::endl;
    close(socket_fd_);
    socket_fd_ = -1;
    return;
  }

  connected_ = true;
  std::cout << "Connected to " << host_ << ":" << port_ << "." << std::endl;
}

void Client::disconnectFromServer() {
  if (socket_fd_ >= 0) {
    close(socket_fd_);
    socket_fd_ = -1;
  }
  if (connected_) {
    std::cout << "Disconnected from server." << std::endl;
    connected_ = false;
  }
}

void Client::sendMessage(const std::string& message) {
  if (socket_fd_ < 0) {
    return;
  }
  ssize_t sent = send(socket_fd_, message.c_str(), message.size(), 0);
  if (sent < 0) {
    std::perror("send");
  }
}

void Client::receiveLoop() {
  char buffer[256];
  while (connected_) {
    ssize_t n = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
      std::cout << "\nServer disconntected" << std::endl;
      connected_ = false;
      break;
    }

    buffer[n] = '\0';
    std::cout << "\r" << buffer;
    std::cout.flush();
  }
}

void Client::run() {
  connectToServer();
  if (!connected_) {
    return;
  }

  char buffer[256];
  ssize_t n = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
  if (n > 0) {
    buffer[n] = '\0';
    std::cout << buffer;
  }

  std::getline(std::cin, username);
  sendMessage(username + "\n");

  recv_thread_ = std::thread(&Client::receiveLoop, this);

  std::cout << "Welcome to the server, " + username + ". Ctrl+C to disconnect" << std::endl;
  std::cout << "[" + username + "]: ";

  std::string line;
  while (connected_ && std::getline(std::cin, line)) {
    std::cout << "[" + username + "]: ";
    if (line.empty()) continue;
    sendMessage(line + "\n");
  }

  stop();
}

void Client::stop() {
  disconnectFromServer();
  if (recv_thread_.joinable()) {
    recv_thread_.join();
  }
}

bool Client::isConnected() const {
  return connected_;
}