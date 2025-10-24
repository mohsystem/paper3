
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
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

const int PORT = 8888;
const int BUFFER_SIZE = 1024;

class ClientHandler;
std::vector<ClientHandler*> clients;
std::mutex clients_mutex;

void broadcastMessage(const std::string& message, ClientHandler* sender);
void removeClient(ClientHandler* client);

class ClientHandler {
private:
    SOCKET clientSocket;
    std::string username;
    
public:
    ClientHandler(SOCKET socket) : clientSocket(socket), username("") {}
    
    void run() {
        char buffer[BUFFER_SIZE];
        
        sendMessage("Enter your username:\\n");
        int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\\0';
            username = std::string(buffer);
            username.erase(username.find_last_not_of("\\r\\n") + 1);
            
            if (username.empty()) {
                username = "Anonymous";
            }
            
            std::cout << username << " joined the chat" << std::endl;
            broadcastMessage(username + " joined the chat", this);
            
            while (true) {
                bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
                
                if (bytesRead <= 0) {
                    break;
                }
                
                buffer[bytesRead] = '\\0';
                std::string message(buffer);
                message.erase(message.find_last_not_of("\\r\\n") + 1);
                
                if (message == "/quit") {
                    break;
                }
                
                std::cout << username << ": " << message << std::endl;
                broadcastMessage(username + ": " + message, this);
            }
        }
        
        cleanup();
    }
    
    void sendMessage(const std::string& message) {
        send(clientSocket, message.c_str(), message.length(), 0);
    }
    
    void cleanup() {
        removeClient(this);
        if (!username.empty()) {
            std::cout << username << " left the chat" << std::endl;
            broadcastMessage(username + " left the chat", this);
        }
        closesocket(clientSocket);
    }
    
    std::string getUsername() const {
        return username;
    }
};

void broadcastMessage(const std::string& message, ClientHandler* sender) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto client : clients) {
        if (client != sender) {
            client->sendMessage(message + "\\n");
        }
    }
}

void removeClient(ClientHandler* client) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
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
    
    std::cout << "Chat Server starting on port " << port << std::endl;
    std::cout << "Server is listening on port " << port << std::endl;
    
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        
        std::cout << "New client connected: " << inet_ntoa(clientAddr.sin_addr) << std::endl;
        
        ClientHandler* handler = new ClientHandler(clientSocket);
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(handler);
        }
        
        std::thread clientThread(&ClientHandler::run, handler);
        clientThread.detach();
    }
    
    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    startServer(PORT);
    return 0;
}
