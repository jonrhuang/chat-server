#include <iostream>
#include "Client.h"

int main(int argc, char** argv) {
  std::string message = argc > 1 ? argv[1] : "Hello from Client";
  Client client;
  client.run(message);
  return 0;
}
