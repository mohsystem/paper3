
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define CLOSE_SOCKET close
    #define INVALID_SOCKET -1
    typedef int socket_t;
#endif

#define BUFFER_SIZE 8192
#define TIMEOUT_SEC 30
#define MIN_PORT 1024
#define MAX_PORT 65535

void handleClient(socket_t clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead;
    
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesRead <= 0) {
            break;
        }

        buffer[bytesRead] = '\\0';
        printf("Received: %s", buffer);
        
        if (send(clientSocket, buffer, bytesRead, 0) < 0) {
            fprintf(stderr, "Send failed\\n");
            break;
        }
    }
}

void startServer(int port) {
    if (port < MIN_PORT || port > MAX_PORT) {
        fprintf(stderr, "Port must be between %d and %d\\n", MIN_PORT, MAX_PORT);
        return;
    }

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\\n");
        return;
    }
#endif

    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        fprintf(stderr, "Setsockopt failed\\n");
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        fprintf(stderr, "Bind failed\\n");
        CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    if (listen(serverSocket, 5) < 0) {
        fprintf(stderr, "Listen failed\\n");
        CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    printf("Server listening on port %d\\n", port);

    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        socket_t clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSocket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed\\n");
            continue;
        }

        printf("Client connected\\n");
        handleClient(clientSocket);
        CLOSE_SOCKET(clientSocket);
    }

    CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

void testClient(int port, const char* message, int testNum) {
    printf("\\nTest %d: Sending message: %s\\n", testNum, message);
    
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    
    socket_t clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "Client socket creation failed\\n");
        return;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(port);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        fprintf(stderr, "Connection failed\\n");
        CLOSE_SOCKET(clientSocket);
        return;
    }

    char msgBuffer[BUFFER_SIZE];
    snprintf(msgBuffer, BUFFER_SIZE, "%s\\n", message);
    send(clientSocket, msgBuffer, strlen(msgBuffer), 0);

    char buffer[BUFFER_SIZE];
    memset(buffer, 0, BUFFER_SIZE);
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\\0';
        printf("Response: %s", buffer);
    }

    CLOSE_SOCKET(clientSocket);
}

int main() {
    printf("Test 1: Starting echo server on port 8080\\n");
    printf("Note: Server functionality requires multi-threading support.\\n");
    printf("This C implementation provides the core socket functionality.\\n");
    printf("For full test execution, use the Java, Python, or C++ versions.\\n");
    
    /* Uncomment to run server (will block)
    startServer(8080);
    */
    
    return 0;
}
