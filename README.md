# Proactor Chat

This project implements Beej's chat using the proactor pattern. The proactor manages the asynchronous I/O operations for handling client connections and sending messages to other chat users.

## Getting Started

To build and run the proactor chat, follow the instructions below.

### Prerequisites

- C++17 compatible compiler (e.g., GCC, Clang)
- GNU Make
- pthread library

### Building the Proactor Library

## Getting Started

1. Clone the repository or download the source code files.

2. Build the project by running the following command:

   ```
   make
   ```

   This will compile the `proact_server` executable and the `libst_proactor.so` library.

3. Specifiy the path to the library :
   ```
   export LD_LIBRARY_PATH="."
   ```
   
4. Run the server by executing the following command:

   ```
   ./react_server
   ```
   

   This command will generate the proactor library file named `libst_proactor.so`.

### Building and Running the Chat

1. Navigate to the `chat` directory:

   ```shell
   cd chat
   ```

2. Build the chat program using the provided Makefile:

   ```shell
   make
   ```

   This command will generate the chat program file named `chat_server`.

3. Start the chat server:

   ```shell
   ./chat_server
   ```

   The chat server will start listening for client connections on port 9034.

4. Open multiple terminal windows or use a tool like `netcat` to simulate multiple clients connecting to the server:

   ```shell
   nc localhost 9034
   ```

   Repeat this command in multiple terminals to simulate multiple chat clients.

5. Start chatting! Type messages in any client terminal, and the messages will be sent to all connected clients.

6. To exit the chat, press `Ctrl+C` in the server terminal.

## Implementation Details

The proactor pattern is used to manage the asynchronous I/O operations in the chat program. The proactor library provides functions to create and manage a proactor, add file descriptors to the proactor, and handle events on those file descriptors.

### Proactor Library

The proactor library is implemented in the `st_proactor.hpp` and `st_proactor.cpp` files. It provides the following functions:

- `void* createProactor()`: Creates a new proactor and returns a pointer to it.

- `int addFD2Proactor(void* thisPtr, int fd, handler_t handler)`: Adds a file descriptor `fd` to the proactor with the specified event handler `handler`.

- `int removeHandler(void* thisPtr, int fd)`: Removes the event handler associated with the file descriptor `fd` from the proactor.

- `int runProactor(void* thisPtr)`: Runs the proactor's event loop, handling events on registered file descriptors.

- `int cancelProactor(void* thisPtr)`: Cancels the proactor and stops its event loop.

### Chat Program

The chat program is implemented in the `proact_server.cpp` file. It utilizes the proactor library to handle client connections and message broadcasting. The main function performs the following steps:

1. Creates an instance of the proactor using the `createProactor` function.

2. Gets a listening socket using the `get_listener_socket` function.

3. Adds the listening socket file descriptor to the proactor using `addFD2Proactor`, with the event handler `handle_listen`.

4. Runs the proactor's event loop using `runProactor`.

5. When a client connects, the `handle_listen` function is invoked. It accepts the client connection, adds the client's socket file descriptor to the proactor using `addFD2Proactor`, with the event handler `handle_clients`, and prints a message about the new client.

6

. When a client sends a message, the `handle_clients` function is invoked. It reads the message from the client socket, creates a new thread to broadcast the message to all other connected clients, and prints a message about the sender client.

7. The `sendMassageToAll` function is responsible for broadcasting the message to all connected clients. It iterates over the file descriptors registered in the proactor and sends the message to all clients except the listener and the sender client.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
