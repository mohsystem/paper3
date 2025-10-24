
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
#define BUFFER_SIZE 1024

class Task48 {
private:
    int server_socket;
    std::vector<int> clients;
    std::mutex clients_mutex;
    
    void broadcast(const std::string& message, int sender_socket) {
        std::cout << message << std::endl;
        std::lock_guard<std::mutex> lock(clients_mutex);
        
        for (int client_socket : clients) {
            std::string msg = message + "\\n";
            send(client_socket, msg.c_str(), msg.length(), 0);
        }
    }
    
    void handle_client(int client_socket) {
        char buffer[BUFFER_SIZE];
        std::string client_name = "Anonymous";
        
        // Get client name
        std::string prompt = "Enter your name: ";
        send(client_socket, prompt.c_str(), prompt.length(), 0);
        
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\\0';
            client_name = std::string(buffer);
            // Remove newline characters
            client_name.erase(std::remove(client_name.begin(), client_name.end(), '\\n'), client_name.end());
            client_name.erase(std::remove(client_name.begin(), client_name.end(), '\\r'), client_name.end());
            
            if (client_name.empty()) {
                client_name = "Anonymous";
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client_socket);
        }
        
        broadcast(client_name + " has joined the chat!", client_socket);
        
        // Handle messages
        while (true) {
            memset(buffer, 0, BUFFER_SIZE);
            bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytes_received <= 0) {
                break;
            }
            
            buffer[bytes_received] = '\\0';
            std::string message(buffer);
            message.erase(std::remove(message.begin(), message.end(), '\\n'), message.end());
            message.erase(std::remove(message.begin(), message.end(), '\\r'), message.end());
            
            if (message == "/quit") {
                break;
            }
            
            broadcast(client_name + ": " + message, client_socket);
        }
        
        // Cleanup
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
        }
        
        broadcast(client_name + " has left the chat!", client_socket);
        close(client_socket);
    }
    
public:
    void start_server(int port) {
        struct sockaddr_in server_addr, client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // Create socket
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket < 0) {
            std::cerr << "Error creating socket" << std::endl;
            return;
        }
        
        // Set socket options
        int opt = 1;
        setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        // Bind socket
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Error binding socket" << std::endl;
            close(server_socket);
            return;
        }
        
        // Listen for connections
        if (listen(server_socket, 5) < 0) {
            std::cerr << "Error listening on socket" << std::endl;
            close(server_socket);
            return;
        }
        
        std::cout << "Chat server started on port " << port << std::endl;
        
        // Accept connections
        while (true) {
            int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
            if (client_socket < 0) {
                std::cerr << "Error accepting connection" << std::endl;
                continue;
            }
            
            std::cout << "New client connected" << std::endl;
            
            // Handle client in a separate thread
            std::thread client_thread(&Task48::handle_client, this, client_socket);
            client_thread.detach();
        }
        
        close(server_socket);
    }
};

int main() {
    std::cout << "=== Chat Server Application ===" << std::endl;
    std::cout << "Test Case 1: Starting server on port " << PORT << std::endl;
    
    Task48 server;
    
    std::cout << "\\nTo test this application:" << std::endl;
    std::cout << "1. Run this server" << std::endl;
    std::cout << "2. Use telnet or netcat to connect: telnet localhost 8888" << std::endl;
    std::cout << "3. Connect multiple clients and send messages" << std::endl;
    std::cout << "4. Type /quit to disconnect" << std::endl;
    
    // Test Cases:
    // 1. Start server - Server should listen on specified port
    // 2. Connect client 1 - Should join and receive welcome message
    // 3. Connect client 2 - Both clients should see join notification
    // 4. Client 1 sends message - All clients should receive broadcast
    // 5. Client disconnects - All clients should see leave notification
    
    server.start_server(PORT);
    
    return 0;
}
