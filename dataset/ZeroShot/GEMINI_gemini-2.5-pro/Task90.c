#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Shared resources
int client_sockets[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *message, int sender_socket) {
    // Secure: Lock the mutex to safely iterate over the shared client_sockets array.
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        // if (client_sockets[i] != sender_socket) {
            if (send(client_sockets[i], message, strlen(message), 0) < 0) {
                perror("send failed");
            }
        // }
    }
    // Secure: Always unlock the mutex after use.
    pthread_mutex_unlock(&clients_mutex);
}

void* handle_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg); // Secure: Free the dynamically allocated memory for the socket descriptor.

    char buffer[BUFFER_SIZE];
    char client_id[100];
    
    // Get client address for identification
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    getpeername(client_socket, (struct sockaddr*)&address, &addrlen);
    sprintf(client_id, "%s:%d", inet_ntoa(address.sin_addr), ntohs(address.sin_port));

    printf("New connection from %s\n", client_id);
    char join_msg[128];
    sprintf(join_msg, "Server: %s has joined.\n", client_id);
    broadcast_message(join_msg, client_socket);

    while (1) {
        // Secure: Clear the buffer before receiving new data.
        memset(buffer, 0, BUFFER_SIZE);
        // Secure: Receive a fixed amount of data to prevent buffer overflows.
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client %s disconnected.\n", client_id);
            } else {
                perror("recv failed");
            }
            break;
        }

        buffer[bytes_received] = '\0'; // Ensure null-termination
        printf("Received from %s: %s", client_id, buffer);
        char broadcast_msg[BUFFER_SIZE + 128];
        sprintf(broadcast_msg, "%s: %s", client_id, buffer);
        broadcast_message(broadcast_msg, client_socket);
    }
    
    // Secure: Resource cleanup. Remove client from shared array.
    char leave_msg[128];
    sprintf(leave_msg, "Server: %s has left.\n", client_id);
    broadcast_message(leave_msg, -1);
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i] == client_socket) {
            // Shift elements to the left to fill the gap.
            for (int j = i; j < client_count - 1; j++) {
                client_sockets[j] = client_sockets[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    
    close(client_socket);
    pthread_exit(NULL);
}

int main() {
    /*
     * How to test this server:
     * 1. Compile this C file (e.g., `gcc -pthread your_file.c -o server`) and run it.
     * 2. Open a terminal and connect: `nc localhost 8080` or `telnet localhost 8080`
     * 3. Open another terminal and connect.
     * 4. Type a message in one client and see it appear in the other.
     * 5. Connect up to 5 clients to simulate 5 test cases.
     * 6. Disconnect a client (Ctrl+C or Ctrl+D) and observe the server's response.
     * NOTE: This code is for POSIX-compliant systems (Linux, macOS). For Windows, you would need to use Winsock and Windows threads.
    */
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;

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

    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1) {
        socklen_t addrlen = sizeof(address);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            continue;
        }

        pthread_mutex_lock(&clients_mutex);
        if (client_count < MAX_CLIENTS) {
            client_sockets[client_count++] = new_socket;
            pthread_mutex_unlock(&clients_mutex);

            pthread_t tid;
            // Secure: Allocate memory for the socket descriptor to avoid race conditions.
            // The new thread is responsible for freeing this memory.
            int *p_socket = malloc(sizeof(int));
            if (p_socket == NULL) {
                perror("malloc failed");
                close(new_socket);
                continue;
            }
            *p_socket = new_socket;

            if (pthread_create(&tid, NULL, handle_client, p_socket) != 0) {
                perror("pthread_create failed");
                free(p_socket);
                close(new_socket);
            }
            pthread_detach(tid); // Allow thread to clean up its resources on exit.
        } else {
            pthread_mutex_unlock(&clients_mutex);
            printf("Max clients reached. Connection rejected.\n");
            send(new_socket, "Server is full. Try again later.\n", 33, 0);
            close(new_socket);
        }
    }
    
    close(server_fd);
    return 0;
}