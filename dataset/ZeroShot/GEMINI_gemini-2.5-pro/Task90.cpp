#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

// Shared resources for all threads
std::vector<int> client_sockets;
std::mutex clients_mutex;

void broadcast_message(const std::string& message, int sender_socket) {
    // Secure: Use a lock guard for RAII-style mutex management to prevent race conditions.
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (int sock : client_sockets) {
        // Broadcast to all clients (can be modified to not send to sender)
        // if (sock != sender_socket) {
            // Secure: Check the return value of send.
            if (send(sock, message.c_str(), message.length(), 0) < 0) {
                perror("send failed");
            }
        // }
    }
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    
    // Get client address for identification
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    getpeername(client_socket, (struct sockaddr*)&address, &addrlen);
    std::string client_id = std::string(inet_ntoa(address.sin_addr)) + ":" + std::to_string(ntohs(address.sin_port));

    std::cout << "New connection from " << client_id << std::endl;
    broadcast_message("Server: " + client_id + " has joined.\n", client_socket);

    while (true) {
        // Secure: Initialize buffer to prevent using old data.
        memset(buffer, 0, BUFFER_SIZE);
        // Secure: Read a fixed amount of data to prevent buffer overflows.
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            // Error or client disconnected
            if (bytes_received == 0) {
                std::cout << "Client " << client_id << " disconnected." << std::endl;
            } else {
                perror("recv failed");
            }
            break; // Exit loop
        }
        
        buffer[bytes_received] = '\0'; // Ensure null-termination
        std::string received_message(buffer);
        std::cout << "Received from " << client_id << ": " << received_message;
        broadcast_message(client_id + ": " + received_message, client_socket);
    }
    
    // Secure: Resource cleanup.
    // Remove client from the list
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto it = client_sockets.begin(); it != client_sockets.end(); ++it) {
            if (*it == client_socket) {
                client_sockets.erase(it);
                break;
            }
        }
    }
    broadcast_message("Server: " + client_id + " has left.\n", -1);
    close(client_socket);
}

int main() {
    /*
     * How to test this server:
     * 1. Compile this C++ file (e.g., `g++ -std=c++11 -pthread your_file.cpp -o server`) and run it.
     * 2. Open a terminal and connect: `nc localhost 8080` or `telnet localhost 8080`
     * 3. Open another terminal and connect.
     * 4. Type a message in one client and see it appear in the other.
     * 5. Connect up to 5 clients to simulate 5 test cases.
     * 6. Disconnect a client (Ctrl+C or Ctrl+D) and observe the server's response.
     * NOTE: This code is for POSIX-compliant systems (Linux, macOS). For Windows, you would need to use Winsock.
    */
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Secure: Allow reuse of address and port to avoid "Address already in use" error on restart.
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // Listen on 0.0.0.0
    address.sin_port = htons(PORT);

    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        int new_socket;
        socklen_t addrlen = sizeof(address);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("accept");
            continue; // Continue to the next iteration
        }
        
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            client_sockets.push_back(new_socket);
        }

        // Create a new thread for each client.
        // Detach the thread to let it run independently. It will manage its own lifecycle.
        std::thread(handle_client, new_socket).detach();
    }
    
    // Secure: Close the listening socket when server shuts down.
    close(server_fd);
    return 0;
}