#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// For Windows, you would include <winsock2.h> and link against ws2_32.lib

const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 1024;

std::vector<int> client_sockets;
std::mutex clients_mutex;

void broadcast_message(const std::string& message, int sender_socket) {
    std::lock_guard<std::mutex> guard(clients_mutex);
    for (int client_socket : client_sockets) {
        if (client_socket != sender_socket) {
            // Use send() with a flag to prevent crashing if the client disconnects
            if (send(client_socket, message.c_str(), message.length(), MSG_NOSIGNAL) < 0) {
                // This indicates a problem with the client socket, but we handle
                // cleanup in the client's own thread.
                perror("send");
            }
        }
    }
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    std::string client_name = "Client-" + std::to_string(client_socket);

    while (true) {
        // Clear buffer before reading
        memset(buffer, 0, BUFFER_SIZE);
        
        // Read data from the client
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                std::cout << client_name << " disconnected gracefully." << std::endl;
            } else {
                perror("recv");
            }
            break; // Exit loop on error or disconnection
        }

        std::string message(buffer, bytes_received);
        std::cout << client_name << " says: " << message << std::endl;

        std::string broadcast_msg = client_name + ": " + message;
        broadcast_message(broadcast_msg, client_socket);
    }

    // Cleanup: Remove client socket from the list and close it
    {
        std::lock_guard<std::mutex> guard(clients_mutex);
        // Safely remove the socket from the vector
        client_sockets.erase(std::remove(client_sockets.begin(), client_sockets.end(), client_socket), client_sockets.end());
    }
    close(client_socket);
    std::cout << "Connection closed for " << client_name << std::endl;
}

void start_server(int port) {
    int server_fd;
    struct sockaddr_in address;
    int opt = 1;

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Binding the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (new_socket < 0) {
            perror("accept");
            continue; // Continue to the next iteration
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Connection accepted from " << client_ip << ":" << ntohs(client_addr.sin_port) << std::endl;
        
        {
            std::lock_guard<std::mutex> guard(clients_mutex);
            if (client_sockets.size() >= MAX_CLIENTS) {
                std::cout << "Max clients reached. Connection rejected." << std::endl;
                close(new_socket);
                continue;
            }
            client_sockets.push_back(new_socket);
        }

        // Create a new thread for each client and detach it
        std::thread(handle_client, new_socket).detach();
    }

    close(server_fd);
}

int main(int argc, char const *argv[]) {
    // "Test cases" for a server involve observing its behavior.
    std::cout << "--- C++ Chat Server ---" << std::endl;
    std::cout << "--- Test Case 1: Starting server ---" << std::endl;
    
    int port = 9090;
    if (argc > 1) {
        try {
            port = std::stoi(argv[1]);
        } catch(const std::exception& e) {
            std::cerr << "Invalid port number. Using default 9090." << std::endl;
        }
    }

    std::cout << "--- Test Case 2: Server will listen on port " << port << std::endl;
    std::cout << "--- Test Case 3: Connect using 'nc localhost " << port << "'" << std::endl;
    std::cout << "--- Test Case 4: Observe connection and message logs." << std::endl;
    std::cout << "--- Test Case 5: The server will handle up to " << MAX_CLIENTS << " concurrent clients." << std::endl;
    
    start_server(port);
    
    return 0;
}