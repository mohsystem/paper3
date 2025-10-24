#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

/**
 * Starts a secure echo server on the specified port.
 * The server handles a limited number of clients sequentially and then shuts down.
 *
 * @param port The port number to listen on.
 * @param maxClients The number of clients to serve before shutting down.
 */
void start_server(int port, int maxClients) {
    int server_fd;
    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        return;
    }

    // Set socket options to allow reusing the address, helpful for quick restarts
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        return;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on all available interfaces
    address.sin_port = htons(port);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return;
    }

    // Start listening for incoming connections
    if (listen(server_fd, 3) < 0) { // Backlog of 3 pending connections
        perror("Listen failed");
        close(server_fd);
        return;
    }

    printf("Server is listening on port %d\n", port);
    printf("Server will handle %d clients and then shut down.\n", maxClients);
    
    for (int i = 0; i < maxClients; ++i) {
        printf("\nWaiting for client %d/%d...\n", i + 1, maxClients);
        
        int client_socket;
        socklen_t addrlen = sizeof(address);
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("Accept failed");
            continue; // Continue to wait for the next client
        }
        
        printf("Client connected.\n");
        
        // Set a receive timeout (10 seconds) to prevent malicious/stalled clients
        struct timeval tv;
        tv.tv_sec = 10;
        tv.tv_usec = 0;
        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv) < 0) {
            perror("setsockopt for timeout failed");
        }
        
        char buffer[4096] = {0};
        ssize_t bytes_read;

        while (1) {
            bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytes_read > 0) {
                // Echo the data back. Use send() and check its return value.
                ssize_t bytes_sent = send(client_socket, buffer, bytes_read, 0);
                if (bytes_sent < 0) {
                    perror("Send failed");
                    break;
                }
                printf("Echoed %zd bytes to client.\n", bytes_sent);
            } else if (bytes_read == 0) {
                // Client closed the connection
                printf("Client disconnected.\n");
                break;
            } else { // bytes_read < 0
                // An error occurred. Check if it was a timeout.
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    fprintf(stderr, "Receive timeout. Closing connection.\n");
                } else {
                    perror("Recv failed");
                }
                break;
            }
        }

        close(client_socket);
    }
    
    close(server_fd);
    printf("Server shutting down after handling %d clients.\n", maxClients);
}

int main() {
    int port = 8080;
    int numberOfTestConnections = 5;

    printf("--- Echo Server Test ---\n");
    printf("To test, connect to this server using a client like netcat or telnet.\n");
    printf("Example 1: nc localhost %d\n", port);
    printf("Example 2: telnet localhost %d\n", port);
    printf("Type a message and press Enter. The server will echo it back.\n");
    printf("The server will shut down after handling %d connections.\n", numberOfTestConnections);
    printf("-------------------------\n");
    
    start_server(port, numberOfTestConnections);

    return 0;
}