#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

// Note: This code is for POSIX-compliant systems (Linux, macOS).
// For Windows, you would need to use the Winsock API.

#define BUFFER_SIZE 1024

/**
 * @brief Starts an echo server that handles one client connection and then exits.
 * @param port The port number to listen on.
 */
void startEchoServer(int port) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Server Error: socket failed");
        exit(EXIT_FAILURE);
    }

    // Set SO_REUSEADDR to allow reusing the port immediately
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Server Warning: setsockopt");
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Server Error: bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("Server Error: listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", port);

    // Accept one incoming connection
    socklen_t addrlen = sizeof(address);
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
        perror("Server Error: accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Client connected.\n");

    // Echo loop
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read;
    while ((bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Ensure null-termination for printing
        printf("Server received: %s", buffer); // No newline, as client sends it
        
        // Echo the message back to the client
        if (send(client_socket, buffer, bytes_read, 0) != bytes_read) {
            perror("Server Error: send failed");
        } else {
             printf("Server echoed message.\n");
        }
        memset(buffer, 0, BUFFER_SIZE); // Clear buffer for next read
    }

    if (bytes_read == 0) {
        printf("Client disconnected.\n");
    } else {
        perror("Server Error: read failed");
    }

    // Close sockets
    close(client_socket);
    close(server_fd);
    printf("Server has shut down.\n");
}

/**
 * @brief Runs 5 test cases by connecting to the server.
 * @param host The IP address of the server.
 * @param port The port number of the server.
 */
void runClientTests(const char* host, int port) {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Client Error: Socket creation error");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 addresses from text to binary form
    if (inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        perror("Client Error: Invalid address / Address not supported");
        close(sock);
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Client Error: Connection Failed");
        close(sock);
        return;
    }
    printf("\nClient connected to server. Running tests...\n");

    const char* test_messages[] = {
        "Hello, Server!\n",
        "This is test case 2.\n",
        "A message with numbers 12345.\n",
        "Another test.\n",
        "Goodbye!\n"
    };
    int num_tests = sizeof(test_messages) / sizeof(test_messages[0]);

    char buffer[BUFFER_SIZE] = {0};
    for (int i = 0; i < num_tests; i++) {
        printf("Client sending: %s", test_messages[i]);
        send(sock, test_messages[i], strlen(test_messages[i]), 0);
        
        ssize_t bytes_read = read(sock, buffer, BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            buffer[bytes_read] = '\0';
            printf("Client received echo: %s", buffer);
        } else {
            fprintf(stderr, "Client Error: Read failed or server disconnected.\n");
            break;
        }
    }

    close(sock);
    printf("Client tests finished.\n");
}

int main() {
    const int PORT = 12348;
    const char* HOST = "127.0.0.1";

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process: The Client
        sleep(1); // Give server a moment to start
        runClientTests(HOST, PORT);
        exit(0);
    } else {
        // Parent process: The Server
        startEchoServer(PORT);
        wait(NULL); // Wait for the client process to finish
    }

    printf("\nProgram finished.\n");
    return 0;
}