#include <iostream>
#include <vector>
#include <string>
#include <cstring>      // For strerror
#include <cerrno>       // For errno
#include <unistd.h>     // For close(), read(), write()
#include <sys/socket.h> // For socket APIs
#include <netinet/in.h> // For sockaddr_in

/**
 * Starts a secure echo server on the specified port.
 * The server handles a limited number of clients sequentially and then shuts down.
 *
 * @param port The port number to listen on.
 * @param maxClients The number of clients to serve before shutting down.
 */
void startServer(int port, int maxClients) {
    int server_fd;
    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return;
    }

    // Set socket options to allow reusing the address, helpful for quick restarts
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    address.sin_port = htons(port);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return;
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) { // Backlog of 3 pending connections
        std::cerr << "Listen failed: " << strerror(errno) << std::endl;
        close(server_fd);
        return;
    }

    std::cout << "Server is listening on port " << port << std::endl;
    std::cout << "Server will handle " << maxClients << " clients and then shut down." << std::endl;

    for (int i = 0; i < maxClients; ++i) {
        std::cout << "\nWaiting for client " << (i + 1) << "/" << maxClients << "..." << std::endl;
        
        int client_socket;
        socklen_t addrlen = sizeof(address);
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            std::cerr << "Accept failed: " << strerror(errno) << std::endl;
            continue; // Continue to wait for the next client
        }

        std::cout << "Client connected." << std::endl;

        // Set a receive timeout (10 seconds) to prevent malicious/stalled clients
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0) {
            std::cerr << "setsockopt for timeout failed: " << strerror(errno) << std::endl;
        }

        // Use a vector for a dynamically-sized, safe buffer
        std::vector<char> buffer(4096);
        ssize_t bytes_read;

        while (true) {
            bytes_read = recv(client_socket, buffer.data(), buffer.size(), 0);

            if (bytes_read > 0) {
                // Echo the data back. Use send() and check its return value.
                ssize_t bytes_sent = send(client_socket, buffer.data(), bytes_read, 0);
                if (bytes_sent < 0) {
                    std::cerr << "Send failed: " << strerror(errno) << std::endl;
                    break;
                }
                 std::cout << "Echoed " << bytes_sent << " bytes to client." << std::endl;
            } else if (bytes_read == 0) {
                // Client closed the connection
                std::cout << "Client disconnected." << std::endl;
                break;
            } else { // bytes_read < 0
                // An error occurred. Check if it was a timeout.
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::cerr << "Receive timeout. Closing connection." << std::endl;
                } else {
                    std::cerr << "Recv failed: " << strerror(errno) << std::endl;
                }
                break;
            }
        }
        
        close(client_socket);
    }

    close(server_fd);
    std::cout << "Server shutting down after handling " << maxClients << " clients." << std::endl;
}

int main() {
    int port = 8080;
    int numberOfTestConnections = 5;

    std::cout << "--- Echo Server Test ---" << std::endl;
    std::cout << "To test, connect to this server using a client like netcat or telnet." << std::endl;
    std::cout << "Example 1: nc localhost " << port << std::endl;
    std::cout << "Example 2: telnet localhost " << port << std::endl;
    std::cout << "Type a message and press Enter. The server will echo it back." << std::endl;
    std::cout << "The server will shut down after handling " << numberOfTestConnections << " connections." << std::endl;
    std::cout << "-------------------------" << std::endl;

    startServer(port, numberOfTestConnections);

    return 0;
}