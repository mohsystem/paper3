#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void handle_client(int client_socket_fd) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    printf("Handling new client connection.\n");

    // Read from client and echo back
    while ((bytes_read = read(client_socket_fd, buffer, BUFFER_SIZE)) > 0) {
        // Write exactly the number of bytes that were read.
        ssize_t bytes_written = write(client_socket_fd, buffer, bytes_read);
        if (bytes_written < 0) {
            perror("write");
            break; // Exit loop on write error
        }
        // Note: For a robust server, you should loop the write call
        // until all 'bytes_read' have been written.
    }

    if (bytes_read < 0) {
        perror("read");
    }

    printf("Client disconnected.\n");
    close(client_socket_fd);
}

void run_server(int port) {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Optional: Reuse the address to avoid "Address already in use" errors
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", port);

    while (1) {
        client_addr_len = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_fd < 0) {
            perror("accept");
            continue; // Don't exit, just wait for the next connection
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        printf("Accepted connection from %s:%d\n", client_ip, ntohs(client_addr.sin_port));

        handle_client(client_fd);
    }

    // This part is unreachable in this simple infinite loop server.
    close(server_fd);
}

int main() {
    int port = 8080;
    
    printf("Starting the echo server...\n");
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