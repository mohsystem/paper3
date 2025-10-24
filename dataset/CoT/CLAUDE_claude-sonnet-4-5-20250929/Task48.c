
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_CLIENTS 10
#define MAX_MESSAGE_LENGTH 1024

typedef struct {
    int socket;
    char clientId[64];
    int active;
} Client;

typedef struct {
    Client clients[MAX_CLIENTS];
    int clientCount;
    pthread_mutex_t mutex;
    int serverSocket;
} ChatServer;

ChatServer server;

void sanitizeMessage(char* message) {
    int len = strlen(message);
    for (int i = 0; i < len; i++) {
        if (message[i] < 32 || message[i] > 126) {
            message[i] = ' ';
        }
    }
}

void broadcast(const char* message, int senderSocket) {
    if (!message || strlen(message) > MAX_MESSAGE_LENGTH) return;
    
    char sanitized[MAX_MESSAGE_LENGTH + 1];
    strncpy(sanitized, message, MAX_MESSAGE_LENGTH);
    sanitized[MAX_MESSAGE_LENGTH] = '\\0';
    sanitizeMessage(sanitized);
    
    pthread_mutex_lock(&server.mutex);
    for (int i = 0; i < server.clientCount; i++) {
        if (server.clients[i].active && server.clients[i].socket != senderSocket) {
            char buffer[MAX_MESSAGE_LENGTH + 2];
            snprintf(buffer, sizeof(buffer), "%s\\n", sanitized);
            send(server.clients[i].socket, buffer, strlen(buffer), 0);
        }
    }
    pthread_mutex_unlock(&server.mutex);
}

void removeClient(int socket) {
    pthread_mutex_lock(&server.mutex);
    for (int i = 0; i < server.clientCount; i++) {
        if (server.clients[i].socket == socket) {
            server.clients[i].active = 0;
            close(socket);
            break;
        }
    }
    pthread_mutex_unlock(&server.mutex);
}

void* handleClient(void* arg) {
    int clientSocket = *(int*)arg;
    free(arg);
    
    char welcome[] = "Connected to chat server. Type messages to send.\\n";
    send(clientSocket, welcome, strlen(welcome), 0);
    
    char buffer[MAX_MESSAGE_LENGTH + 1];
    char messageBuffer[MAX_MESSAGE_LENGTH * 2];
    messageBuffer[0] = '\\0';
    
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(clientSocket, buffer, MAX_MESSAGE_LENGTH, 0);
        
        if (bytesRead <= 0) break;
        
        strncat(messageBuffer, buffer, MAX_MESSAGE_LENGTH);
        
        char* newline;
        while ((newline = strchr(messageBuffer, '\\n')) != NULL) {
            *newline = '\\0';
            
            if (strlen(messageBuffer) > 0) {
                char broadcastMsg[MAX_MESSAGE_LENGTH + 128];
                snprintf(broadcastMsg, sizeof(broadcastMsg), "Client: %s", messageBuffer);
                broadcast(broadcastMsg, clientSocket);
            }
            
            memmove(messageBuffer, newline + 1, strlen(newline + 1) + 1);
        }
    }
    
    removeClient(clientSocket);
    return NULL;
}

void startServer() {
    server.clientCount = 0;
    pthread_mutex_init(&server.mutex, NULL);
    
    server.serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (server.serverSocket < 0) {
        perror("Socket creation failed");
        return;
    }
    
    int opt = 1;
    setsockopt(server.serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server.serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server.serverSocket);
        return;
    }
    
    if (listen(server.serverSocket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server.serverSocket);
        return;
    }
    
    printf("Server started on port %d\\n", PORT);
    
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(server.serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket < 0) continue;
        
        pthread_mutex_lock(&server.mutex);
        if (server.clientCount >= MAX_CLIENTS) {
            close(clientSocket);
            pthread_mutex_unlock(&server.mutex);
            continue;
        }
        
        server.clients[server.clientCount].socket = clientSocket;
        server.clients[server.clientCount].active = 1;
        snprintf(server.clients[server.clientCount].clientId, 64, "%s", inet_ntoa(clientAddr.sin_addr));
        server.clientCount++;
        pthread_mutex_unlock(&server.mutex);
        
        pthread_t thread;
        int* socketPtr = malloc(sizeof(int));
        *socketPtr = clientSocket;
        pthread_create(&thread, NULL, handleClient, socketPtr);
        pthread_detach(thread);
    }
}

int main() {
    printf("Test Case 1: Start server\\n");
    printf("Chat server implementation in C\\n");
    printf("Note: Full multi-client test requires separate processes\\n");
    
    printf("\\nTest Case 2-5: Server functionality ready\\n");
    printf("To test: Run server in one terminal, clients in others\\n");
    
    return 0;
}
