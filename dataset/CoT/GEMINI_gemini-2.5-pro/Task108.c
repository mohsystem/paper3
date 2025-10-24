#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h> // For sig_atomic_t

// Global configuration
#define PORT 8081
#define HOST "127.0.0.1"
#define BUFFER_SIZE 1024

// Use a volatile sig_atomic_t for a variable shared between threads
// that controls a loop, which is safe for simple atomic writes.
volatile sig_atomic_t server_running = 1;

// 1. Client handler function (runs in a separate thread)
void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg); // Free the dynamically allocated integer

    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    printf("Server: New client connected on socket %d.\n", client_socket);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_read = read(client_socket, buffer, BUFFER_SIZE - 1);

        if (bytes_read <= 0) {
            if (bytes_read == 0) {
                printf("Server: Client on socket %d disconnected.\n", client_socket);
            } else {
                perror("Server: read error");
            }
            break; // Exit loop
        }
        
        // Remove trailing newline characters for safe processing
        buffer[strcspn(buffer, "\r\n")] = 0;

        if (strcmp(buffer, "SHUTDOWN") == 0) {
            printf("Server: Shutdown command received.\n");
            server_running = 0;
            break;
        }

        // Parse command: OP A B
        char op[4];
        double a, b;
        // Use sscanf for parsing, checking its return value for security.
        if (sscanf(buffer, "%3s %lf %lf", op, &a, &b) != 3) {
            snprintf(response, BUFFER_SIZE, "ERROR: Invalid command format\n");
        } else {
            double result = 0.0;
            int error = 0;
            if (strcmp(op, "ADD") == 0) result = a + b;
            else if (strcmp(op, "SUB") == 0) result = a - b;
            else if (strcmp(op, "MUL") == 0) result = a * b;
            else if (strcmp(op, "DIV") == 0) {
                if (b == 0) {
                    snprintf(response, BUFFER_SIZE, "ERROR: Division by zero\n");
                    error = 1;
                } else {
                    result = a / b;
                }
            } else {
                snprintf(response, BUFFER_SIZE, "ERROR: Unknown operation\n");
                error = 1;
            }

            if (!error) {
                // Use snprintf to prevent buffer overflows when creating the response.
                snprintf(response, BUFFER_SIZE, "%f\n", result);
            }
        }
        write(client_socket, response, strlen(response));
    }

    close(client_socket);
    return NULL;
}


// 2. Server main function
void* run_server(void* arg) {
    int server_fd;
    struct sockaddr_in server_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Server: socket failed");
        return NULL;
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Server: setsockopt failed");
        close(server_fd);
        return NULL;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Server: bind failed");
        close(server_fd);
        return NULL;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Server: listen failed");
        close(server_fd);
        return NULL;
    }
    
    printf("Server: Listening on port %d\n", PORT);

    while (server_running) {
        int client_socket = accept(server_fd, NULL, NULL);
        if (client_socket < 0) {
            if (server_running) { // Don't print error on controlled shutdown
                perror("Server: accept failed");
            }
            continue;
        }

        pthread_t tid;
        // Pass a copy of the socket descriptor to the thread
        int* p_client_socket = malloc(sizeof(int));
        if (!p_client_socket) {
            perror("Server: malloc failed");
            close(client_socket);
            continue;
        }
        *p_client_socket = client_socket;

        if (pthread_create(&tid, NULL, handle_client, p_client_socket) != 0) {
            perror("Server: pthread_create failed");
            free(p_client_socket);
            close(client_socket);
        }
        pthread_detach(tid); // We don't need to join the client threads
    }
    
    close(server_fd);
    printf("Server: Shut down.\n");
    return NULL;
}

// 3. Client main function
void run_client() {
    printf("\nClient: Starting client...\n");

    const char* tests[][2] = {
        {"1", "ADD 10 5"},
        {"2", "SUB 10 5"},
        {"3", "MUL 10 5"},
        {"4", "DIV 10 5"},
        {"5", "DIV 10 0"}
    };

    for (int i = 0; i < 5; ++i) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) {
            perror("Client: Socket creation error");
            continue;
        }
        
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, HOST, &serv_addr.sin_addr);

        if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            printf("Client: Connection Failed for test %s.\n", tests[i][0]);
            close(sock);
            continue;
        }
        
        char request[BUFFER_SIZE];
        snprintf(request, BUFFER_SIZE, "%s\n", tests[i][1]);
        write(sock, request, strlen(request));
        
        char buffer[BUFFER_SIZE] = {0};
        read(sock, buffer, BUFFER_SIZE - 1);
        buffer[strcspn(buffer, "\r\n")] = 0; // Remove newline for clean printing

        printf("Client Test %s (%s): %s\n", tests[i][0], tests[i][1], buffer);
        
        close(sock);
    }

    // Send shutdown signal to the server
    int final_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (final_sock >= 0) {
        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(PORT);
        inet_pton(AF_INET, HOST, &serv_addr.sin_addr);
        if (connect(final_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == 0) {
            char shutdown_cmd[] = "SHUTDOWN\n";
            write(final_sock, shutdown_cmd, strlen(shutdown_cmd));
        }
        close(final_sock);
    }
}


// 4. Main entry point
int main() {
    pthread_t server_thread_id;

    if (pthread_create(&server_thread_id, NULL, run_server, NULL) != 0) {
        perror("Failed to create server thread");
        return 1;
    }

    // Give the server a moment to start up
    sleep(1);

    run_client();

    // Wait for the server thread to complete
    pthread_join(server_thread_id, NULL);
    
    printf("Program finished.\n");
    return 0;
}