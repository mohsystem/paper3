#include <iostream>
#include <vector>
#include <cstring>      // For memset
#include <unistd.h>     // For read, write, close
#include <sys/socket.h> // For socket APIs
#include <netinet/in.h> // For sockaddr_in
#include <arpa/inet.h>  // For inet_ntop

void handle_client(int client_socket_fd) {
    // The client socket will be closed at the end of this function.
    std::cout << "Handling new client connection." << std::endl;

    const size_t BUFFER_SIZE = 1024;
    std::vector<char> buffer(BUFFER_SIZE);
    ssize_t bytes_read;

    // Read from client and echo back until the connection is closed or an error occurs.
    while ((bytes_read = read(client_socket_fd, buffer.data(), BUFFER_SIZE)) > 0) {
        // Echo the data back. Write exactly the number of bytes that were read.
        ssize_t bytes_written = write(client_socket_fd, buffer.data(), bytes_read);
        if (bytes_written < 0) {
            perror("write");
            break; // Stop on write error
        }
        // Note: A robust implementation might loop 'write' to ensure all data is sent.
    }

    if (bytes_read < 0) {
        perror("read");
    }

    std::cout << "Client disconnected." << std::endl;
    close(client_socket_fd);
}

void run_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }

    // Optional: Reuse the address to avoid "Address already in use" errors.
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return;
    }

    struct sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }

    std::cout << "Server is listening on port " << port << std::endl;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        
        if (client_fd < 0) {
            perror("accept");
            continue; // Continue to the next attempt to accept
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Accepted connection from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;

        handle_client(client_fd);
    }
    
    // Unreachable in this infinite loop design. A signal handler would be needed for graceful shutdown.
    close(server_fd);
}

int main() {
    int port = 8080;

    std::cout << "Starting the echo server..." << std::endl;
    // The prompt asks for 5 test cases. A server runs continuously.
    // To test, you can use a client like netcat (nc) or telnet from 5 different
    // terminals, or sequentially after each one disconnects.
    //
    // Example test cases using netcat from a terminal:
    // Test Case 1: echo "Hello Server" | nc localhost 8080
    // Test Case 2: echo "This is a test" | nc localhost 8080
    // Test Case 3: echo "Another line" | nc localhost 8080
    // Test Case 4: echo "A short one" | nc localhost 8080
    // Test Case 5: echo "Final test message" | nc localhost 8080

    run_server(port);
    
    return 0;
}