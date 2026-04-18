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
  if (connected_) {
    return;
  }

  socket_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_fd_ < 0) {
    std::error_code ec(errno, std::generic_category());
    std::cerr << "Socket creating failed: " << ec.message() << std::endl;
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
    std::error_code ec(errno, std::generic_category());
    std::cerr << "Could not connect to server: " << ec.message() << std::endl;
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

std::string Client::receiveMessage() {
  if (socket_fd_ < 0) {
    return {};
  }

  char buffer[256];
  ssize_t received = recv(socket_fd_, buffer, sizeof(buffer) - 1, 0);
  if (received <= 0) {
    return {};
  }

  buffer[received] = '\0';
  return std::string(buffer);
}

void Client::run(std::string message) {
  connectToServer();
  if (!connected_) {
    return;
  }

  sendMessage(message + "\n");
  std::string response = receiveMessage();
  if (!response.empty()) {
    std::cout << "Server response: " << response;
  }

  stop();
}

void Client::stop() {
  disconnectFromServer();
}

bool Client::isConnected() const {
  return connected_;
}