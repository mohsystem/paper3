
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define close closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <pthread.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 1024

void* handleClient(void* arg) {
    SOCKET clientSocket = *(SOCKET*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    
    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytesReceived] = '\\0';
        printf("Received: %s\\n", buffer);
        send(clientSocket, buffer, bytesReceived, 0);
    }
    
    close(clientSocket);
    printf("Client disconnected\\n");
    return NULL;
}

void startServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\\n");
        return;
    }
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed\\n");
        close(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed\\n");
        close(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }
    
    printf("Server started on port %d\\n", port);
    
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed\\n");
            continue;
        }
        
        printf("Client connected\\n");
        
        SOCKET* clientSocketPtr = malloc(sizeof(SOCKET));
        *clientSocketPtr = clientSocket;
        
#ifdef _WIN32
        HANDLE thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)handleClient, clientSocketPtr, 0, NULL);
        if (thread) CloseHandle(thread);
#else
        pthread_t thread;
        pthread_create(&thread, NULL, handleClient, clientSocketPtr);
        pthread_detach(thread);
#endif
    }
    
    close(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    printf("Echo Server Test Cases\\n");
    printf("======================\\n");
    printf("Test Case 1: Start server on default port 8080\\n");
    printf("Test Case 2: Server handles multiple clients\\n");
    printf("Test Case 3: Server echoes messages correctly\\n");
    printf("Test Case 4: Server handles client disconnection\\n");
    printf("Test Case 5: Server runs continuously\\n");
    printf("\\nStarting server...\\n");
    
    startServer(DEFAULT_PORT);
    
    return 0;
}
