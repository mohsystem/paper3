#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
typedef int SOCKET;
const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
#define closesocket close
#endif

const int PORT = 12345;
const int MAX_CLIENTS = 10;
const int BUFFER_SIZE = 1024;

std::vector<SOCKET> clients;
std::mutex clients_mutex;
int client_counter = 0;

void broadcast_message(const std::string& message, SOCKET sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (SOCKET client_socket : clients) {
        if (client_socket != sender_socket) {
            if (send(client_socket, message.c_str(), message.length(), 0) == SOCKET_ERROR) {
                // Error handling can be improved to remove disconnected clients here
            }
        }
    }
}

void handle_client(SOCKET client_socket, int client_id) {
    std::cout << "Client-" << client_id << " connected." << std::endl;
    broadcast_message("Client-" + std::to_string(client_id) + " has joined the chat.", client_socket);

    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::string message = "[Client-" + std::to_string(client_id) + "]: " + buffer;
            std::cout << "Broadcasting: " << message;
            broadcast_message(message, client_socket);
        } else {
            // Error or connection closed
            break;
        }
    }

    std::cout << "Client-" << client_id << " disconnected." << std::endl;
    
    // Remove client from the list
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    }

    broadcast_message("Client-" + std::to_string(client_id) + " has left the chat.", client_socket);
    closesocket(client_socket);
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return 1;
    }
#endif

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(server_fd);
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(server_fd);
        return 1;
    }

    std::cout << "C++ Chat Server listening on port " << PORT << std::endl;

    while (true) {
        sockaddr_in client_addr{};
        int client_addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t*)&client_addr_len);

        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }

        std::unique_lock<std::mutex> lock(clients_mutex);
        if (clients.size() >= MAX_CLIENTS) {
            lock.unlock();
            std::cout << "Connection refused: maximum clients reached." << std::endl;
            const char* msg = "Server is full. Please try again later.\n";
            send(client_socket, msg, strlen(msg), 0);
            closesocket(client_socket);
        } else {
            clients.push_back(client_socket);
            int id = ++client_counter;
            lock.unlock();
            std::thread(handle_client, client_socket, id).detach();
        }
    }

    closesocket(server_fd);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
    
    // A main method is provided to run the server.
    // To test, you can use a netcat or telnet client.
    // For example, in 5 separate terminals, run:
    // 1. Terminal 1: nc localhost 12345
    // 2. Terminal 2: nc localhost 12345
    // 3. Terminal 3: nc localhost 12345
    // 4. Terminal 4: nc localhost 12345
    // 5. Terminal 5: nc localhost 12345
    // Type messages in any terminal, and they should appear in all others.
}