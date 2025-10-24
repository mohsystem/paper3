#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <errno.h>

#define PORT 12345
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int clients[MAX_CLIENTS];
int client_count = 0;
int next_client_id = 1;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *message, int sender_fd, int message_len) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender_fd) {
            if (send(clients[i], message, message_len, 0) < 0) {
                perror("send failed");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void* handle_client(void* arg) {
    int client_fd = *((int*)arg);
    int client_id = *((int*)arg + 1);
    free(arg);

    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 50];

    printf("Client-%d connected.\n", client_id);
    snprintf(message, sizeof(message), "Client-%d has joined the chat.\n", client_id);
    broadcast_message(message, client_fd, strlen(message));

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            if(strlen(buffer) > 0 && buffer[strlen(buffer)-1] == '\n') {
                buffer[strlen(buffer)-1] = '\0';
            }
             if(strlen(buffer) > 0 && buffer[strlen(buffer)-1] == '\r') {
                buffer[strlen(buffer)-1] = '\0';
            }
            if (strlen(buffer) == 0) continue;

            snprintf(message, sizeof(message), "[Client-%d]: %s\n", client_id, buffer);
            printf("Broadcasting: %s", message);
            broadcast_message(message, client_fd, strlen(message));
        } else {
            if (bytes_received == 0) {
                printf("Client-%d disconnected.\n", client_id);
            } else {
                perror("recv failed");
            }
            break;
        }
    }

    // Remove client
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_fd) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    snprintf(message, sizeof(message), "Client-%d has left the chat.\n", client_id);
    broadcast_message(message, -1, strlen(message)); // Send to all
    close(client_fd);
    return NULL;
}

int main() {
    int server_fd;
    struct sockaddr_in server_addr;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("C Chat Server listening on port %d\n", PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (new_socket < 0) {
            perror("accept failed");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (client_count < MAX_CLIENTS) {
            clients[client_count++] = new_socket;
            int client_id = next_client_id++;
            pthread_mutex_unlock(&clients_mutex);

            int* p_client_info = malloc(2 * sizeof(int));
            if (p_client_info == NULL) {
                 perror("malloc failed");
                 close(new_socket);
                 continue;
            }
            p_client_info[0] = new_socket;
            p_client_info[1] = client_id;

            pthread_t tid;
            if (pthread_create(&tid, NULL, handle_client, p_client_info) != 0) {
                perror("pthread_create failed");
                free(p_client_info);
                close(new_socket);
            }
            pthread_detach(tid);
        } else {
            pthread_mutex_unlock(&clients_mutex);
            printf("Connection refused: maximum clients reached.\n");
            const char *msg = "Server is full. Please try again later.\n";
            send(new_socket, msg, strlen(msg), 0);
            close(new_socket);
        }
    }

    close(server_fd);
    return 0;
    // A main method is provided to run the server.
    // To test, you can use a netcat or telnet client.
    // For example, in 5 separate terminals, run:
    // 1. Terminal 1: nc localhost 12345
    // 2. Terminal 2: nc localhost 12345
    // 3. Terminal 3: nc localhost 12345
    // 4. Terminal 4: nc localhost 12345
    // 5. Terminal 5: nc localhost 12345
    // Type messages in any terminal, and they should appear in all others.
}