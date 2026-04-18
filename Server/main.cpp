#include <thread>
#include <iostream>
#include "Server.h"

int main() {
    Server server;

    std::thread serverThread([&] {
        server.run();
    });

    std::cout << "Press Enter to stop the server...\n";
    std::cin.get();

    server.stop();
    serverThread.join();

    return 0;
}