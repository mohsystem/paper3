#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

// Globals for client management
int client_sockets[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] != sender_socket) {
            // Use send() with MSG_NOSIGNAL to avoid crashing on broken pipe
            if (send(client_sockets[i], message, strlen(message), MSG_NOSIGNAL) < 0) {
                perror("send");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    int i;
    for (i = 0; i < client_count; i++) {
        if (client_sockets[i] == client_socket) {
            break;
        }
    }
    // Shift remaining elements left
    if (i < client_count) {
        for (int j = i; j < client_count - 1; j++) {
            client_sockets[j] = client_sockets[j + 1];
        }
        client_count--;
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    free(arg); // Free the allocated memory for the socket descriptor
    char buffer[BUFFER_SIZE];
    char message_to_broadcast[BUFFER_SIZE + 50];

    int bytes_received;
    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received string
        printf("Client %d says: %s\n", client_socket, buffer);

        // Prepare message for broadcasting
        snprintf(message_to_broadcast, sizeof(message_to_broadcast), "Client %d: %s", client_socket, buffer);
        broadcast_message(message_to_broadcast, client_socket);

        // Clear buffer
        memset(buffer, 0, BUFFER_SIZE);
    }

    if (bytes_received == 0) {
        printf("Client %d disconnected.\n", client_socket);
    } else {
        perror("recv");
    }

    // Cleanup
    remove_client(client_socket);
    close(client_socket);
    pthread_exit(NULL);
}


void start_server(int port) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Initialize client sockets array
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_sockets[i] = 0;
    }

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
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        printf("New connection, socket fd is %d, ip is: %s, port: %d\n", 
               new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        pthread_mutex_lock(&clients_mutex);
        if (client_count < MAX_CLIENTS) {
            client_sockets[client_count++] = new_socket;
            pthread_t thread_id;
            // Pass a dynamically allocated integer to the new thread
            int *p_client_socket = malloc(sizeof(int));
            if (p_client_socket == NULL) {
                perror("malloc failed");
                close(new_socket);
                client_count--;
                pthread_mutex_unlock(&clients_mutex);
                continue;
            }
            *p_client_socket = new_socket;

            if (pthread_create(&thread_id, NULL, handle_client, (void *)p_client_socket) != 0) {
                perror("pthread_create failed");
                free(p_client_socket);
                client_count--;
            }
        } else {
            printf("Max clients reached. Connection rejected.\n");
            close(new_socket);
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    close(server_fd);
}


int main(int argc, char *argv[]) {
    int port = 9090;

    if (argc > 1) {
        port = atoi(argv[1]);
        if (port == 0) {
            fprintf(stderr, "Invalid port number. Using default 9090.\n");
            port = 9090;
        }
    }

    printf("--- C Chat Server ---\n");
    printf("--- Test Case 1: Initializing server resources.\n");
    printf("--- Test Case 2: Server will start on port %d.\n", port);
    printf("--- Test Case 3: Use 'nc localhost %d' to connect.\n", port);
    printf("--- Test Case 4: Up to %d clients can connect simultaneously.\n", MAX_CLIENTS);
    printf("--- Test Case 5: Logs will show client connections, messages, and disconnections.\n");

    start_server(port);

    return 0;
}