#pragma once

#include <atomic>
#include <mutex>
#include <vector>
#include <string>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

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
  std::vector<int> clients_;
  std::mutex clients_mutex_;
  mongocxx::instance mongo_instance_;
  mongocxx::client mongo_client_;
  mongocxx::database db_;
  mongocxx::collection messages_col_;


  void init();
  void cleanup();
  bool createSocket();
  void acceptLoop();
  void handleClient(int clientSocket);
  void broadcastMessage(const std::string& message, int senderFd);
  void persistMessage(const std::string& username, const std::string& message);
};