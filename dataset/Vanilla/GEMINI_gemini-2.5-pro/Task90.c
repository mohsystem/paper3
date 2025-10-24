#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define close_socket(s) closesocket(s)
    // Note: This C code uses pthreads, which is not native on Windows.
    // You'd need a library like pthreads-win32 to compile this on Windows.
    // The code is primarily intended for POSIX-compliant systems like Linux or macOS.
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #define close_socket(s) close(s)
#endif

#define MAX_CLIENTS 100
#define BUFFER_SIZE 4096

// Shared resources for all client threads
int clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    int socket_fd;
    char name[INET_ADDRSTRLEN + 6]; // IP:PORT
} client_info_t;

// Function to broadcast a message to all clients except the sender
void broadcast_message(const char* message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] != sender_socket) {
            if (send(clients[i], message, strlen(message), 0) < 0) {
                perror("send failed");
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Function to handle a single client connection
void* handle_client(void* arg) {
    client_info_t* info = (client_info_t*)arg;
    int client_socket = info->socket_fd;
    char client_name[sizeof(info->name)];
    strcpy(client_name, info->name);
    free(info);

    printf("New client connected: %s\n", client_name);

    // Add client to the list
    pthread_mutex_lock(&clients_mutex);
    if (client_count < MAX_CLIENTS) {
        clients[client_count++] = client_socket;
    }
    pthread_mutex_unlock(&clients_mutex);
    
    char join_msg[100];
    snprintf(join_msg, sizeof(join_msg), "Server: %s has joined the chat.\n", client_name);
    printf("%s", join_msg);
    broadcast_message(join_msg, client_socket);

    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytes_received] = '\0';
        if (strncmp(buffer, "exit", 4) == 0) {
            break;
        }

        char broadcast_msg[BUFFER_SIZE + 100];
        snprintf(broadcast_msg, sizeof(broadcast_msg), "%s: %s", client_name, buffer);
        printf("Received: %s", broadcast_msg);
        broadcast_message(broadcast_msg, client_socket);
    }

    // Client disconnected
    char leave_msg[100];
    snprintf(leave_msg, sizeof(leave_msg), "Server: %s has left the chat.\n", client_name);
    printf("%s", leave_msg);
    broadcast_message(leave_msg, client_socket);
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] == client_socket) {
            // Remove by shifting the last element here
            clients[i] = clients[client_count - 1];
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);

    close_socket(client_socket);
    return NULL;
}

// Main server function
void run_server(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        exit(EXIT_FAILURE);
    }
#endif

    int server_socket;
    struct sockaddr_in server_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close_socket(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) < 0) {
        perror("listen failed");
        close_socket(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("C Chat Server is running on port %d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        
        if (client_socket < 0) {
            perror("accept failed");
            continue;
        }
        
        client_info_t* client_info = malloc(sizeof(client_info_t));
        if (!client_info) {
            perror("malloc failed");
            close_socket(client_socket);
            continue;
        }

        client_info->socket_fd = client_socket;
        inet_ntop(AF_INET, &client_addr.sin_addr, client_info->name, INET_ADDRSTRLEN);
        char port_str[6];
        snprintf(port_str, sizeof(port_str), ":%d", ntohs(client_addr.sin_port));
        strcat(client_info->name, port_str);

        pthread_t tid;
        if (pthread_create(&tid, NULL, handle_client, (void*)client_info) != 0) {
            perror("pthread_create failed");
            free(client_info);
            close_socket(client_socket);
        }
        pthread_detach(tid);
    }

    close_socket(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    printf("--- C Server Test ---\n");
    printf("How to Test:\n");
    printf("1. Compile and run this program.\n");
    printf("   - On Linux/macOS: gcc -o server_c Task90.c -pthread\n");
    printf("   - On Windows: Requires a POSIX threads library like pthreads-win32.\n");
    printf("2. Run the executable: ./server_c\n");
    printf("3. Open 5 separate terminal/command prompt windows.\n");
    printf("4. In each terminal, connect using telnet or netcat:\n");
    printf("   telnet localhost 12345\n");
    printf("5. Type messages and press Enter to broadcast.\n");
    printf("6. To quit a client, type 'exit' or close the terminal.\n");
    printf("7. To stop the server, use Ctrl+C in its terminal.\n");
    printf("--------------------------\n");
    
    run_server(12345);

    return 0;
}