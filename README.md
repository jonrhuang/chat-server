# chat-server

A simple C++ chat server and client example using POSIX sockets.

This project contains two programs:

- `server`: listens for incoming TCP connections and responds to a single client message.
- `client`: connects to the server, sends a message, receives a response, then closes the connection.

## Project Structure

- `Makefile` - build rules for compiling the server and client.
- `Server/` - server source files.
- `Client/` - client source files.

## Requirements

- Linux or UNIX-like OS
- `g++` compiler
- POSIX sockets support

## Build

From the project root folder, run:

```bash
make
```

That builds two executables:

- `server`
- `client`

## Run

1. Start the server:

```bash
./server
```

2. In another terminal, run the client:

```bash
./client
```

The client sends a default message: `Hello from Client`.

## Custom Client Message

To send a custom message, pass it as a command-line argument:

```bash
./client "Hello from my custom client"
```

## Cleanup

To remove the compiled executables:

```bash
make clean
```

## Notes

- The server runs until you press Enter in the server terminal.
- The client connects once, sends one message, receives the response, and exits.

