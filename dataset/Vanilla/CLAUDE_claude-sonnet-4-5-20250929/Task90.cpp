
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

class Task90 {
private:
    struct ClientHandler {
        socket_t socket;
        int clientId;
        std::string username;
        Task90* server;
        
        ClientHandler(socket_t s, int id, Task90* srv) 
            : socket(s), clientId(id), username("User" + std::to_string(id)), server(srv) {}
        
        void run() {
            char buffer[1024];
            
            std::string welcome = "Welcome to the chat! You are " + username + "\\n";
            send(socket, welcome.c_str(), welcome.length(), 0);
            server->broadcastMessage(username + " has joined the chat", this);
            
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int bytesReceived = recv(socket, buffer, sizeof(buffer) - 1, 0);
                
                if (bytesReceived <= 0) {
                    break;
                }
                
                std::string message(buffer);
                message.erase(message.find_last_not_of("\\r\\n") + 1);
                
                std::cout << username << ": " << message << std::endl;
                server->broadcastMessage(username + ": " + message, this);
            }
            
            cleanup();
        }
        
        void sendMessage(const std::string& message) {
            std::string msg = message + "\\n";
            send(socket, msg.c_str(), msg.length(), 0);
        }
        
        void cleanup() {
            server->removeClient(this);
            server->broadcastMessage(username + " has left the chat", this);
            closesocket(socket);
        }
    };
    
    std::vector<ClientHandler*> clients;
    std::mutex clientsMutex;
    int clientCounter;

public:
    Task90() : clientCounter(0) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    }
    
    ~Task90() {
        for (auto client : clients) {
            delete client;
        }
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    void startServer(int port) {
        socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);
        
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed" << std::endl;
            closesocket(serverSocket);
            return;
        }
        
        if (listen(serverSocket, 5) == SOCKET_ERROR) {
            std::cerr << "Listen failed" << std::endl;
            closesocket(serverSocket);
            return;
        }
        
        std::cout << "Chat server started on port " << port << std::endl;
        
        while (true) {
            sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            socket_t clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
            
            if (clientSocket == INVALID_SOCKET) {
                continue;
            }
            
            clientCounter++;
            ClientHandler* handler = new ClientHandler(clientSocket, clientCounter, this);
            
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clients.push_back(handler);
            }
            
            std::thread(&ClientHandler::run, handler).detach();
        }
        
        closesocket(serverSocket);
    }
    
    void broadcastMessage(const std::string& message, ClientHandler* sender) {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (auto client : clients) {
            if (client != sender) {
                client->sendMessage(message);
            }
        }
    }
    
    void removeClient(ClientHandler* client) {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
    }
};

void testClient(int clientNum, int port) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200 * clientNum));
    
    socket_t clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        return;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
    
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(clientSocket);
        return;
    }
    
    char buffer[1024];
    recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    std::cout << "Client " << clientNum << " received: " << buffer;
    
    std::string msg1 = "Hello from client " + std::to_string(clientNum) + "\\n";
    send(clientSocket, msg1.c_str(), msg1.length(), 0);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::string msg2 = "Message 2 from client " + std::to_string(clientNum) + "\\n";
    send(clientSocket, msg2.c_str(), msg2.length(), 0);
    
    for (int i = 0; i < 10; i++) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytes > 0) {
            std::cout << "Client " << clientNum << " received: " << buffer;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    closesocket(clientSocket);
}

int main() {
    std::cout << "Test Case 1: Starting server on port 7777" << std::endl;
    
    Task90 server;
    std::thread serverThread(&Task90::startServer, &server, 7777);
    serverThread.detach();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::vector<std::thread> clientThreads;
    for (int i = 1; i <= 5; i++) {
        clientThreads.emplace_back(testClient, i, 7777);
    }
    
    for (auto& thread : clientThreads) {
        thread.join();
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "\\nTest completed successfully!" << std::endl;
    
    return 0;
}
