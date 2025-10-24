
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

#define PORT 8888
#define MAX_CLIENTS 100
#define MAX_MESSAGE_LENGTH 1024
#define BUFFER_SIZE 2048

std::vector<int> clients;
std::mutex clients_mutex;

std::string sanitizeMessage(const std::string& message) {
    std::string sanitized;
    for (char c : message) {
        if ((c >= 32 && c <= 126) || c == '\\n' || c == '\\r' || c == '\\t') {
            sanitized += c;
        }
    }
    if (sanitized.length() > MAX_MESSAGE_LENGTH) {
        sanitized = sanitized.substr(0, MAX_MESSAGE_LENGTH);
    }
    return sanitized;
}

void broadcast(const std::string& message, int sender_socket) {
    if (message.length() > MAX_MESSAGE_LENGTH) {
        return;
    }
    
    std::string sanitized = sanitizeMessage(message);
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    for (int client_socket : clients) {
        if (client_socket != sender_socket) {
            send(client_socket, sanitized.c_str(), sanitized.length(), MSG_NOSIGNAL);
        }
    }
}

void removeClient(int client_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
}

void handleClient(int client_socket, std::string client_address) {
    char buffer[BUFFER_SIZE];
    std::string client_id = client_address;
    
    std::string welcome = "Welcome to the chat server!\\n";
    send(client_socket, welcome.c_str(), welcome.length(), MSG_NOSIGNAL);
    
    std::string join_msg = client_id + " joined the chat\\n";
    broadcast(join_msg, client_socket);
    
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (bytes_received <= 0) {
            break;
        }
        
        std::string message(buffer, bytes_received);
        message = sanitizeMessage(message);
        
        if (!message.empty() && message != "\\n" && message != "\\r\\n") {
            std::cout << client_id << ": " << message;
            if (message.back() != '\\n') {
                std::cout << std::endl;
            }
            
            std::string broadcast_msg = client_id + ": " + message;
            if (broadcast_msg.back() != '\\n') {
                broadcast_msg += "\\n";
            }
            broadcast(broadcast_msg, client_socket);
        }
    }
    
    removeClient(client_socket);
    close(client_socket);
    
    std::string leave_msg = client_id + " left the chat\\n";
    broadcast(leave_msg, -1);
}

void startServer(int port) {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
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
        std::cerr << "Bind failed" << std::endl;
        close(server_socket);
        return;
    }
    
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        std::cerr << "Listen failed" << std::endl;
        close(server_socket);
        return;
    }
    
    std::cout << "Server started. Waiting for clients on port " << port << "..." << std::endl;
    
    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        
        if (client_socket < 0) {
            continue;
        }
        
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            if (clients.size() >= MAX_CLIENTS) {
                close(client_socket);
                continue;
            }
            clients.push_back(client_socket);
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::string client_address = std::string(client_ip) + ":" + std::to_string(ntohs(client_addr.sin_port));
        
        std::cout << "New client connected: " << client_address << std::endl;
        
        std::thread client_thread(handleClient, client_socket, client_address);
        client_thread.detach();
    }
    
    close(server_socket);
}

int main() {
    std::cout << "Chat Server Starting on port " << PORT << std::endl;
    startServer(PORT);
    return 0;
}
