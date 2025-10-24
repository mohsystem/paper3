
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define MAX_CLIENTS 100
#define MAX_MESSAGE_LENGTH 1024
#define BUFFER_SIZE 2048

typedef struct {
    int socket;
    char address[64];
} ClientInfo;

int clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void sanitizeMessage(char* message, size_t max_len) {
    size_t len = strlen(message);
    size_t j = 0;
    
    for (size_t i = 0; i < len && j < max_len - 1; i++) {
        if ((message[i] >= 32 && message[i] <= 126) || 
            message[i] == '\\n' || message[i] == '\\r' || message[i] == '\\t') {
            message[j++] = message[i];
        }
    }
    message[j] = '\\0';
}

void broadcast(const char* message, int sender_socket) {
    if (strlen(message) > MAX_MESSAGE_LENGTH) {
        return;
    }
    
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] != sender_socket && clients[i] > 0) {
            send(clients[i], message, strlen(message), MSG_NOSIGNAL);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void removeClient(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i] == client_socket) {
            for (int j = i; j < client_count - 1; j++) {
                clients[j] = clients[j + 1];
            }
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void* handleClient(void* arg) {
    ClientInfo* client_info = (ClientInfo*)arg;
    int client_socket = client_info->socket;
    char* client_address = client_info->address;
    
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE];
    
    snprintf(message, sizeof(message), "Welcome to the chat server!\\n");
    send(client_socket, message, strlen(message), MSG_NOSIGNAL);
    
    snprintf(message, sizeof(message), "%s joined the chat\\n", client_address);
    broadcast(message, client_socket);
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            break;
        }
        
        sanitizeMessage(buffer, BUFFER_SIZE);
        
        if (strlen(buffer) > 0 && strcmp(buffer, "\\n") != 0 && strcmp(buffer, "\\r\\n") != 0) {
            printf("%s: %s", client_address, buffer);
            if (buffer[strlen(buffer) - 1] != '\\n') {
                printf("\\n");
            }
            
            snprintf(message, sizeof(message), "%s: %s", client_address, buffer);
            if (message[strlen(message) - 1] != '\\n') {
                strcat(message, "\\n");
            }
            broadcast(message, client_socket);
        }
    }
    
    removeClient(client_socket);
    close(client_socket);
    
    snprintf(message, sizeof(message), "%s left the chat\\n", client_address);
    broadcast(message, -1);
    
    free(client_info);
    return NULL;
}

void startServer(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Failed to create socket");
        return;
    }
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);
    
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return;
    }
    
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket);
        return;
    }
    
    printf("Server started. Waiting for clients on port %d...\\n", port);
    
    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            continue;
        }
        
        pthread_mutex_lock(&clients_mutex);
        if (client_count >= MAX_CLIENTS) {
            pthread_mutex_unlock(&clients_mutex);
            close(client_socket);
            continue;
        }
        clients[client_count++] = client_socket;
        pthread_mutex_unlock(&clients_mutex);
        
        ClientInfo* client_info = malloc(sizeof(ClientInfo));
        client_info->socket = client_socket;
        snprintf(client_info->address, sizeof(client_info->address), "%s:%d",
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        
        printf("New client connected: %s\\n", client_info->address);
        
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, handleClient, client_info);
        pthread_detach(thread_id);
    }
    
    close(server_socket);
}

int main() {
    printf("Chat Server Starting on port %d\\n", PORT);
    startServer(PORT);
    return 0;
}
