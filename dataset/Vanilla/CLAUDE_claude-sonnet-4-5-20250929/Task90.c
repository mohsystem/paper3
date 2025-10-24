
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024

typedef struct {
    socket_t socket;
    int clientId;
    char username[32];
    int active;
} ClientInfo;

typedef struct {
    ClientInfo clients[MAX_CLIENTS];
    int clientCount;
    int clientCounter;
    pthread_mutex_t mutex;
} ChatServer;

ChatServer server;

void broadcastMessage(const char* message, socket_t senderSocket) {
    pthread_mutex_lock(&server.mutex);
    for (int i = 0; i < server.clientCount; i++) {
        if (server.clients[i].active && server.clients[i].socket != senderSocket) {
            char msg[BUFFER_SIZE + 2];
            snprintf(msg, sizeof(msg), "%s\\n", message);
            send(server.clients[i].socket, msg, strlen(msg), 0);
        }
    }
    pthread_mutex_unlock(&server.mutex);
}

void removeClient(socket_t clientSocket) {
    pthread_mutex_lock(&server.mutex);
    for (int i = 0; i < server.clientCount; i++) {
        if (server.clients[i].socket == clientSocket) {
            server.clients[i].active = 0;
            break;
        }
    }
    pthread_mutex_unlock(&server.mutex);
}

void* handleClient(void* arg) {
    ClientInfo* clientInfo = (ClientInfo*)arg;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + 64];
    
    snprintf(buffer, sizeof(buffer), "Welcome to the chat! You are %s\\n", clientInfo->username);
    send(clientInfo->socket, buffer, strlen(buffer), 0);
    
    snprintf(message, sizeof(message), "%s has joined the chat", clientInfo->username);
    broadcastMessage(message, clientInfo->socket);
    
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientInfo->socket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived <= 0) {
            break;
        }
        
        buffer[strcspn(buffer, "\\r\\n")] = 0;
        
        printf("%s: %s\\n", clientInfo->username, buffer);
        
        snprintf(message, sizeof(message), "%s: %s", clientInfo->username, buffer);
        broadcastMessage(message, clientInfo->socket);
    }
    
    snprintf(message, sizeof(message), "%s has left the chat", clientInfo->username);
    broadcastMessage(message, clientInfo->socket);
    removeClient(clientInfo->socket);
    close(clientInfo->socket);
    
    return NULL;
}

void startServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    
    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\\n");
        return;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\\n");
        close(serverSocket);
        return;
    }
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\\n");
        close(serverSocket);
        return;
    }
    
    printf("Chat server started on port %d\\n", port);
    
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        socket_t clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket == INVALID_SOCKET) {
            continue;
        }
        
        pthread_mutex_lock(&server.mutex);
        if (server.clientCount < MAX_CLIENTS) {
            server.clientCounter++;
            server.clients[server.clientCount].socket = clientSocket;
            server.clients[server.clientCount].clientId = server.clientCounter;
            snprintf(server.clients[server.clientCount].username, 32, "User%d", server.clientCounter);
            server.clients[server.clientCount].active = 1;
            
            pthread_t thread;
            pthread_create(&thread, NULL, handleClient, &server.clients[server.clientCount]);
            pthread_detach(thread);
            
            server.clientCount++;
        }
        pthread_mutex_unlock(&server.mutex);
    }
    
    close(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

void* testClient(void* arg) {
    int clientNum = *((int*)arg);
    free(arg);
    
#ifdef _WIN32
    Sleep(200 * clientNum);
#else
    usleep(200000 * clientNum);
#endif
    
    socket_t clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        return NULL;
    }
    
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6666);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        close(clientSocket);
        return NULL;
    }
    
    char buffer[BUFFER_SIZE];
    recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    printf("Client %d received: %s", clientNum, buffer);
    
    snprintf(buffer, sizeof(buffer), "Hello from client %d\\n", clientNum);
    send(clientSocket, buffer, strlen(buffer), 0);
    
#ifdef _WIN32
    Sleep(500);
#else
    usleep(500000);
#endif
    
    snprintf(buffer, sizeof(buffer), "Message 2 from client %d\\n", clientNum);
    send(clientSocket, buffer, strlen(buffer), 0);
    
    for (int i = 0; i < 10; i++) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            printf("Client %d received: %s", clientNum, buffer);
        }
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }
    
    close(clientSocket);
    return NULL;
}

int main() {
    printf("Test Case 1: Starting server on port 6666\\n");
    
    server.clientCount = 0;
    server.clientCounter = 0;
    pthread_mutex_init(&server.mutex, NULL);
    
    pthread_t serverThread;
    int port = 6666;
    pthread_create(&serverThread, NULL, (void*(*)(void*))startServer, (void*)(long)port);
    pthread_detach(serverThread);
    
#ifdef _WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
    
    pthread_t clientThreads[5];
    for (int i = 1; i <= 5; i++) {
        int* clientNum = malloc(sizeof(int));
        *clientNum = i;
        pthread_create(&clientThreads[i-1], NULL, testClient, clientNum);
    }
    
    for (int i = 0; i < 5; i++) {
        pthread_join(clientThreads[i], NULL);
    }
    
#ifdef _WIN32
    Sleep(2000);
#else
    sleep(2);
#endif
    
    printf("\\nTest completed successfully!\\n");
    
    pthread_mutex_destroy(&server.mutex);
    
    return 0;
}
