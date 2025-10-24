
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cerrno>
#include <limits>

// Security: Validate port range to prevent invalid socket configuration
bool isValidPort(int port) {
    return port > 0 && port <= 65535;
}

// Security: Safe buffer size with maximum limit to prevent excessive memory usage
const size_t MAX_BUFFER_SIZE = 65536;  // 64KB limit
const size_t INITIAL_BUFFER_SIZE = 4096;

// Security: Echo server with input validation and bounds checking
int runEchoServer(int port) {
    // Security: Validate port parameter
    if (!isValidPort(port)) {
        std::cerr << "Error: Invalid port number. Must be 1-65535." << std::endl;
        return -1;
    }

    // Security: Create socket with error checking
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error: Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }

    // Security: Set SO_REUSEADDR to avoid "Address already in use" errors
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error: setsockopt failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    // Security: Initialize address structure with zeros to prevent info leaks
    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(static_cast<uint16_t>(port));

    // Security: Bind socket with error checking
    if (bind(server_fd, reinterpret_cast<struct sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "Error: Bind failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    // Security: Listen with reasonable backlog
    if (listen(server_fd, 5) < 0) {
        std::cerr << "Error: Listen failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    std::cout << "Server listening on port " << port << std::endl;

    // Security: Accept client connection with validation
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    std::memset(&client_addr, 0, sizeof(client_addr));
    
    int client_fd = accept(server_fd, reinterpret_cast<struct sockaddr*>(&client_addr), &client_len);
    if (client_fd < 0) {
        std::cerr << "Error: Accept failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return -1;
    }

    // Security: Allocate buffer with bounds checking
    char* buffer = new(std::nothrow) char[INITIAL_BUFFER_SIZE];
    if (buffer == nullptr) {
        std::cerr << "Error: Memory allocation failed" << std::endl;
        close(client_fd);
        close(server_fd);
        return -1;
    }

    // Security: Initialize buffer to prevent info leaks
    std::memset(buffer, 0, INITIAL_BUFFER_SIZE);

    // Security: Read with size validation and bounds checking
    ssize_t bytes_read = recv(client_fd, buffer, INITIAL_BUFFER_SIZE - 1, 0);
    if (bytes_read < 0) {
        std::cerr << "Error: Receive failed: " << strerror(errno) << std::endl;
        // Security: Clear sensitive data before freeing
        std::memset(buffer, 0, INITIAL_BUFFER_SIZE);
        delete[] buffer;
        close(client_fd);
        close(server_fd);
        return -1;
    }

    // Security: Null-terminate received data to prevent buffer overread
    buffer[bytes_read] = '\\0';

    std::cout << "Received: " << buffer << std::endl;

    // Security: Echo back with size validation
    if (bytes_read > 0) {
        ssize_t bytes_sent = send(client_fd, buffer, static_cast<size_t>(bytes_read), 0);
        if (bytes_sent < 0) {
            std::cerr << "Error: Send failed: " << strerror(errno) << std::endl;
        } else {
            std::cout << "Echoed " << bytes_sent << " bytes back to client" << std::endl;
        }
    }

    // Security: Clear buffer before freeing to prevent data leaks
    std::memset(buffer, 0, INITIAL_BUFFER_SIZE);
    delete[] buffer;
    
    // Security: Close sockets in reverse order
    close(client_fd);
    close(server_fd);

    return 0;
}

int main() {
    // Test cases
    std::cout << "=== Test Case 1: Valid port 8080 ===" << std::endl;
    runEchoServer(8080);

    std::cout << "\\n=== Test Case 2: Valid port 9000 ===" << std::endl;
    runEchoServer(9000);

    std::cout << "\\n=== Test Case 3: Invalid port 0 ===" << std::endl;
    runEchoServer(0);

    std::cout << "\\n=== Test Case 4: Invalid port -1 ===" << std::endl;
    runEchoServer(-1);

    std::cout << "\\n=== Test Case 5: Invalid port 70000 ===" << std::endl;
    runEchoServer(70000);

    return 0;
}
