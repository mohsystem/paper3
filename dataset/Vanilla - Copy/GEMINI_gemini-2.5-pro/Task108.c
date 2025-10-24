#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// For networking - POSIX compliant (Linux, macOS)
// For Windows, you would need to include <winsock2.h> and link against ws2_32.lib
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8889
#define BUFFER_SIZE 1024

// --- Server Logic ---
void handle_client_c(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    char command[10];
    double a, b, result;

    read(client_socket, buffer, BUFFER_SIZE);
    
    int items_scanned = sscanf(buffer, "%s %lf %lf", command, &a, &b);

    if (items_scanned != 3) {
        snprintf(response, BUFFER_SIZE, "Error: Invalid request format.");
    } else {
        if (strcmp(command, "ADD") == 0) {
            result = a + b;
            snprintf(response, BUFFER_SIZE, "%f", result);
        } else if (strcmp(command, "SUB") == 0) {
            result = a - b;
            snprintf(response, BUFFER_SIZE, "%f", result);
        } else if (strcmp(command, "MUL") == 0) {
            result = a * b;
            snprintf(response, BUFFER_SIZE, "%f", result);
        } else if (strcmp(command, "DIV") == 0) {
            if (b == 0) {
                snprintf(response, BUFFER_SIZE, "Error: Division by zero.");
            } else {
                result = a / b;
                snprintf(response, BUFFER_SIZE, "%f", result);
            }
        } else {
            snprintf(response, BUFFER_SIZE, "Error: Unknown command.");
        }
    }
    
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}

void run_server_c() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server listening on port %d\n", PORT);
    
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            continue;
        }
        handle_client_c(new_socket);
    }
}

// --- Client Logic ---
void send_request_c(const char* request) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed. Is the server running? \n");
        return;
    }

    send(sock, request, strlen(request), 0);
    read(sock, buffer, BUFFER_SIZE - 1);
    printf("%s\n", buffer);
    close(sock);
}

void run_client_c() {
    printf("Client connected to server.\n");
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Addition
    printf("Test 1: 5 + 3 = ");
    send_request_c("ADD 5 3");
    
    // Test Case 2: Subtraction
    printf("Test 2: 10 - 4 = ");
    send_request_c("SUB 10 4");

    // Test Case 3: Multiplication
    printf("Test 3: 7 * 6 = ");
    send_request_c("MUL 7 6");
    
    // Test Case 4: Division
    printf("Test 4: 20 / 5 = ");
    send_request_c("DIV 20 5");

    // Test Case 5: Division by zero
    printf("Test 5: 10 / 0 -> ");
    send_request_c("DIV 10 0");
}

/**
 * Main function to launch server or client.
 * NOTE: This code uses POSIX sockets. For Windows, use Winsock.
 * --- How to Run ---
 * 1. Compile: gcc -o task108_c task108.c
 * 2. Start Server in one terminal: ./task108_c server
 * 3. Start Client in another terminal: ./task108_c client
 */
int main(int argc, char* argv[]) {
    // There is no Task108 class in C. The main function serves as the entry point.
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server|client>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[1], "server") == 0) {
        run_server_c();
    } else if (strcmp(argv[1], "client") == 0) {
        run_client_c();
    } else {
        fprintf(stderr, "Invalid mode. Use 'server' or 'client'.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}