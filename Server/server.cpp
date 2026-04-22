#include "Server.h"

#include <arpa/inet.h>
#include <chrono>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <algorithm>

Server::Server(int port)
  : port_(port), 
    running_(false), 
    listen_fd_(-1), 
    mongo_instance_{},
    mongo_client_{mongocxx::uri{"mongodb://localhost:27017"}},
    db_{mongo_client_["chat-server"]},
    messages_col_{db_["messages"]} {
  init();
}

Server::~Server() {
  cleanup();
}

void Server::init() {
  std::cout << "Initializing server on port " << port_ << "..." << std::endl;
  if (!createSocket()) {
    std::cerr << "Failed to create server socket." << std::endl;
  }
}

bool Server::createSocket() {
  listen_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ < 0) {
    std::error_code ec(errno, std::generic_category());
    std::cerr << "Could create socket: " << ec.message() << std::endl;
    return false;
  }

  int yes = 1;
  if (setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0) {
    std::error_code ec(errno, std::generic_category());
    std::cerr << "Could not set socket reuse settings: " << ec.message() << std::endl;
    close(listen_fd_);
    listen_fd_ = -1;
    return false;
  }

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port_);

  if (bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    std::error_code ec(errno, std::generic_category());
    std::cerr << "Could not bind to socket: " << ec.message() << std::endl;
    close(listen_fd_);
    listen_fd_ = -1;
    return false;
  }

  if (listen(listen_fd_, 10) < 0) {
    std::error_code ec(errno, std::generic_category());
    std::cerr << "Error listening: " << ec.message() << std::endl;
    close(listen_fd_);
    listen_fd_ = -1;
    return false;
  }

  std::cout << "Server listening on port " << port_ << "." << std::endl;
  return true;
}

void Server::cleanup() {
  if (running_) {
    stop();
  }

  if (listen_fd_ >= 0) {
    close(listen_fd_);
    listen_fd_ = -1;
  }

  std::cout << "Server cleanup complete." << std::endl;
}

void Server::run() {
  if (listen_fd_ < 0) {
    std::cerr << "Server socket is not ready. Cannot run." << std::endl;
    return;
  }

  running_ = true;
  std::cout << "Server running on port " << port_ << "." << std::endl;
  acceptLoop();
  stop();
}

void Server::acceptLoop() {
  while (running_) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(listen_fd_, &readfds);

    timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int ready = select(listen_fd_ + 1, &readfds, nullptr, nullptr, &timeout);
    if (ready < 0) {
      if (errno == EINTR) {
        continue;
      }
      std::cerr << "Select error" << std::endl;
      break;
    }

    if (ready == 0) {
      continue;
    }

    if (FD_ISSET(listen_fd_, &readfds)) {
      int clientSocket = accept(listen_fd_, nullptr, nullptr);
      if (clientSocket < 0) {
        if (errno == EINTR) {
          continue;
        }
        std::cerr << "Accept error" << std::endl;
        continue;
      }

      {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_.push_back(clientSocket);
      }

      std::thread([this, clientSocket]() {
        handleClient(clientSocket);
      }).detach();
    }
  }
}

void Server::handleClient(int clientSocket) {
  std::cout << "Client connected." << std::endl;

  const char* prompt = "Enter your username: ";
  send(clientSocket, prompt, strlen(prompt), 0);

  char buffer[256];
  std::string username;

  ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
  if (bytesRead > 0) {
    buffer[bytesRead] = '\0';
    username = std::string(buffer);

    if (!username.empty() && username.back() == '\n') username.pop_back();
    if (!username.empty() && username.back() == '\r') username.pop_back();
  }

  std::string joinMsg = username + " has joined the chat\n";
  std::cout << joinMsg;
  broadcastMessage(joinMsg, clientSocket);

  while (running_) {
    ssize_t n = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0) {
      break;
    }

    buffer[n] = '\0';
    std::string raw(buffer);
    if (!raw.empty() && raw.back() == '\n') raw.pop_back();
    if (!raw.empty() && raw.back() == '\r') raw.pop_back();

    std::string message = "[" + username + "]: " + raw + "\n";
    std::cout << message;
    broadcastMessage(message, clientSocket);
    persistMessage(username, raw);
  }

  std::string leaveMsg = username + " has left the chat\n";
  std::cout << leaveMsg;
  broadcastMessage(leaveMsg, clientSocket);

  {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    clients_.erase(std::remove(clients_.begin(), clients_.end(), clientSocket), clients_.end());
  }
  close(clientSocket);
}

void Server::broadcastMessage(const std::string& message, int senderFd) {
  std::lock_guard<std::mutex> lock(clients_mutex_);
  for (int fd: clients_) {
    if (fd != senderFd) {
      send(fd, message.c_str(), message.size(), 0);
    }
  }
}

void Server::persistMessage(const std::string& username, const std::string& message) {
  try {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      now.time_since_epoch()).count();

    auto doc = bsoncxx::builder::stream::document{}
      << "username"  << username
      << "message"   << message
      << "timestamp" << bsoncxx::types::b_date{std::chrono::milliseconds{timestamp}}
      << bsoncxx::builder::stream::finalize;

    messages_col_.insert_one(doc.view());
  } catch (const std::exception& e) {
    std::cerr << "MongoDB insert failed: " << e.what() << std::endl;
  }
}

void Server::stop() {
  if (running_) {
    running_ = false;
    std::cout << "Server stopping..." << std::endl;
  }
}

bool Server::isRunning() const {
  return running_;
}