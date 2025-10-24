
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <vector>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

const int DEFAULT_PORT = 8080;
const int BUFFER_SIZE = 1024;

void handleClient(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;
    
    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytesReceived] = '\\0';
        std::cout << "Received: " << buffer << std::endl;
        send(clientSocket, buffer, bytesReceived, 0); // Echo back
    }
    
    closesocket(clientSocket);
    std::cout << "Client disconnected" << std::endl;
}

void startServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return;
    }
#endif

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }
    
    std::cout << "Server started on port " << port << std::endl;
    
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        
        std::cout << "Client connected" << std::endl;
        std::thread(handleClient, clientSocket).detach();
    }
    
    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    std::cout << "Echo Server Test Cases" << std::endl;
    std::cout << "======================" << std::endl;
    std::cout << "Test Case 1: Start server on default port 8080" << std::endl;
    std::cout << "Test Case 2: Server handles multiple clients" << std::endl;
    std::cout << "Test Case 3: Server echoes messages correctly" << std::endl;
    std::cout << "Test Case 4: Server handles client disconnection" << std::endl;
    std::cout << "Test Case 5: Server runs continuously" << std::endl;
    std::cout << "\\nStarting server..." << std::endl;
    
    startServer(DEFAULT_PORT);
    
    return 0;
}
