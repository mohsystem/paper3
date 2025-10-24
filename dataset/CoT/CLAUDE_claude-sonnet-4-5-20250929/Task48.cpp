
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 12345
#define MAX_CLIENTS 10
#define MAX_MESSAGE_LENGTH 1024

class ClientHandler;

class ChatServer {
private:
    std::vector<ClientHandler*> clients;
    std::mutex clientsMutex;
    int serverSocket;
    
public:
    ChatServer() : serverSocket(-1) {}
    
    void start();
    void broadcast(const std::string& message, ClientHandler* sender);
    void removeClient(ClientHandler* client);
    void shutdown();
    
private:
    std::string sanitizeMessage(const std::string& message);
};

class ClientHandler {
private:
    int socket;
    ChatServer* server;
    std::string clientId;
    
public:
    ClientHandler(int sock, const std::string& id, ChatServer* srv)
        : socket(sock), clientId(id), server(srv) {}
    
    void handle();
    void sendMessage(const std::string& message);
    void cleanup();
    int getSocket() const { return socket; }
};

std::string ChatServer::sanitizeMessage(const std::string& message) {
    std::string sanitized;
    for (char c : message) {
        if (c >= 32 && c <= 126) {
            sanitized += c;
        }
    }
    return sanitized;
}

void ChatServer::start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close(serverSocket);
        return;
    }
    
    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(serverSocket);
        return;
    }
    
    std::cout << "Server started on port " << PORT << std::endl;
    
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket < 0) continue;
        
        std::lock_guard<std::mutex> lock(clientsMutex);
        if (clients.size() >= MAX_CLIENTS) {
            close(clientSocket);
            continue;
        }
        
        std::string clientId = inet_ntoa(clientAddr.sin_addr);
        ClientHandler* handler = new ClientHandler(clientSocket, clientId, this);
        clients.push_back(handler);
        
        std::thread([handler]() { handler->handle(); }).detach();
    }
}

void ChatServer::broadcast(const std::string& message, ClientHandler* sender) {
    if (message.empty() || message.length() > MAX_MESSAGE_LENGTH) return;
    
    std::string sanitized = sanitizeMessage(message);
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    for (auto* client : clients) {
        if (client != sender) {
            client->sendMessage(sanitized);
        }
    }
}

void ChatServer::removeClient(ClientHandler* client) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());
}

void ChatServer::shutdown() {
    if (serverSocket >= 0) {
        close(serverSocket);
    }
}

void ClientHandler::handle() {
    std::string welcome = "Connected to chat server. Type messages to send.\\n";
    send(socket, welcome.c_str(), welcome.length(), 0);
    server->broadcast(clientId + " joined the chat", this);
    
    char buffer[MAX_MESSAGE_LENGTH + 1];
    std::string messageBuffer;
    
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = recv(socket, buffer, MAX_MESSAGE_LENGTH, 0);
        
        if (bytesRead <= 0) break;
        
        messageBuffer += std::string(buffer, bytesRead);
        size_t pos;
        
        while ((pos = messageBuffer.find('\\n')) != std::string::npos) {
            std::string message = messageBuffer.substr(0, pos);
            messageBuffer = messageBuffer.substr(pos + 1);
            
            if (message.length() > MAX_MESSAGE_LENGTH) {
                message = message.substr(0, MAX_MESSAGE_LENGTH);
            }
            
            if (!message.empty()) {
                server->broadcast(clientId + ": " + message, this);
            }
        }
    }
    
    cleanup();
}

void ClientHandler::sendMessage(const std::string& message) {
    std::string msg = message + "\\n";
    send(socket, msg.c_str(), msg.length(), 0);
}

void ClientHandler::cleanup() {
    server->removeClient(this);
    server->broadcast(clientId + " left the chat", this);
    close(socket);
    delete this;
}

class ChatClient {
private:
    int socket;
    
public:
    ChatClient() : socket(-1) {}
    
    bool connect(const std::string& host, int port) {
        socket = ::socket(AF_INET, SOCK_STREAM, 0);
        if (socket < 0) return false;
        
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
        
        if (::connect(socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            close(socket);
            return false;
        }
        
        std::thread([this]() {
            char buffer[MAX_MESSAGE_LENGTH + 1];
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int bytesRead = recv(socket, buffer, MAX_MESSAGE_LENGTH, 0);
                if (bytesRead <= 0) break;
                std::cout << buffer;
            }
        }).detach();
        
        return true;
    }
    
    void sendMessage(const std::string& message) {
        if (socket >= 0 && !message.empty() && message.length() <= MAX_MESSAGE_LENGTH) {
            std::string msg = message + "\\n";
            send(socket, msg.c_str(), msg.length(), 0);
        }
    }
    
    void disconnect() {
        if (socket >= 0) {
            close(socket);
            socket = -1;
        }
    }
};

int main() {
    std::cout << "Test Case 1: Start server" << std::endl;
    ChatServer server;
    std::thread serverThread([&server]() { server.start(); });
    serverThread.detach();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "\\nTest Case 2: Connect client 1" << std::endl;
    ChatClient client1;
    client1.connect("127.0.0.1", PORT);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "\\nTest Case 3: Connect client 2" << std::endl;
    ChatClient client2;
    client2.connect("127.0.0.1", PORT);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "\\nTest Case 4: Send messages" << std::endl;
    client1.sendMessage("Hello from client 1");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client2.sendMessage("Hello from client 2");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "\\nTest Case 5: Disconnect clients" << std::endl;
    client1.disconnect();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client2.disconnect();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    return 0;
}
