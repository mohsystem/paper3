#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>

// For POSIX sockets
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring> // For memset and strerror
#include <cerrno>  // For errno

// Note: This code is for POSIX-compliant systems (Linux, macOS).
// For Windows, you would need to include <winsock2.h> and call WSAStartup().

/**
 * @brief Starts an echo server that handles one client connection and then exits.
 * @param port The port number to listen on.
 */
void startEchoServer(int port) {
    // Create socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Server Error: Failed to create socket. " << strerror(errno) << std::endl;
        return;
    }

    // Optional: Set SO_REUSEADDR to allow reusing the port immediately
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Server Warning: setsockopt(SO_REUSEADDR) failed. " << strerror(errno) << std::endl;
    }

    // Bind socket to an address
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Server Error: Bind failed. " << strerror(errno) << std::endl;
        close(server_fd);
        return;
    }

    // Listen for incoming connections
    if (listen(server_fd, 1) < 0) {
        std::cerr << "Server Error: Listen failed. " << strerror(errno) << std::endl;
        close(server_fd);
        return;
    }
    std::cout << "Server is listening on port " << port << std::endl;

    // Accept a connection
    sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_socket = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
    if (client_socket < 0) {
        std::cerr << "Server Error: Accept failed. " << strerror(errno) << std::endl;
        close(server_fd);
        return;
    }
    std::cout << "Client connected." << std::endl;

    // Echo loop
    char buffer[1024] = {0};
    ssize_t bytes_read;
    while ((bytes_read = read(client_socket, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate for printing
        std::cout << "Server received: " << buffer; // No newline, as client sends it
        send(client_socket, buffer, bytes_read, 0);
        std::cout << "Server echoed message." << std::endl;
        memset(buffer, 0, sizeof(buffer)); // Clear buffer
    }

    if (bytes_read == 0) {
        std::cout << "Client disconnected." << std::endl;
    } else {
        std::cerr << "Server Error: Read failed. " << strerror(errno) << std::endl;
    }

    // Clean up
    close(client_socket);
    close(server_fd);
    std::cout << "Server has shut down." << std::endl;
}

/**
 * @brief Runs 5 test cases by connecting to the server.
 * @param host The hostname or IP of the server.
 * @param port The port number of the server.
 */
void runClientTests(const char* host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Client Error: Socket creation failed." << std::endl;
        return;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY; // Connect to localhost

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Client Error: Connection Failed. " << strerror(errno) << std::endl;
        close(sock);
        return;
    }
    std::cout << "\nClient connected to server. Running tests..." << std::endl;

    std::vector<std::string> testMessages = {
        "Hello, Server!\n",
        "This is test case 2.\n",
        "A message with numbers 12345.\n",
        "Another test.\n",
        "Goodbye!\n"
    };

    char buffer[1024] = {0};
    for (const auto& msg : testMessages) {
        std::cout << "Client sending: " << msg;
        send(sock, msg.c_str(), msg.length(), 0);
        
        ssize_t bytes_read = read(sock, buffer, sizeof(buffer) - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            std::cout << "Client received echo: " << buffer;
        } else {
            std::cerr << "Client Error: Read failed or server disconnected." << std::endl;
            break;
        }
    }

    close(sock);
    std::cout << "Client tests finished." << std::endl;
}

int main() {
    const int PORT = 12347;
    const char* HOST = "127.0.0.1";

    // Start the server in a new thread
    std::thread serverThread(startEchoServer, PORT);

    // Give the server a moment to start up
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    // Run client tests from the main thread
    runClientTests(HOST, PORT);

    // Wait for the server thread to finish
    serverThread.join();

    std::cout << "\nProgram finished." << std::endl;

    return 0;
}