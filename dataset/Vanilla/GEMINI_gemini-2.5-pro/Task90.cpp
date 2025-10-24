#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <algorithm>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    #define close_socket(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    using socket_t = int;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define close_socket(s) close(s)
#endif

// Shared resources for all client threads
std::vector<socket_t> clients;
std::mutex clients_mutex;

// Function to broadcast a message to all clients except the sender
void broadcast_message(const std::string& message, socket_t sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (socket_t client_socket : clients) {
        if (client_socket != sender_socket) {
            send(client_socket, message.c_str(), message.length(), 0);
        }
    }
}

// Function to handle a single client connection
void handle_client(socket_t client_socket, std::string client_name) {
    std::cout << "New client connected: " << client_name << std::endl;

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_socket);
    }
    
    std::string join_msg = "Server: " + client_name + " has joined the chat.\n";
    std::cout << join_msg;
    broadcast_message(join_msg, client_socket);

    char buffer[4096];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        std::string message(buffer, bytes_received);
        if (message.rfind("exit", 0) == 0) { // Check if message starts with "exit"
            break;
        }
        std::string broadcast_msg = client_name + ": " + message;
        std::cout << "Received: " << broadcast_msg;
        broadcast_message(broadcast_msg, client_socket);
    }

    // Client disconnected
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    }

    std::string leave_msg = "Server: " + client_name + " has left the chat.\n";
    std::cout << leave_msg;
    broadcast_message(leave_msg, INVALID_SOCKET); // Send to all remaining clients

    close_socket(client_socket);
}

// Main server function
void run_server(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif

    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket." << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        close_socket(server_socket);
        return;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        close_socket(server_socket);
        return;
    }

    std::cout << "CPP Chat Server is running on port " << port << std::endl;

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_addr_size = sizeof(client_addr);
        socket_t client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }
        
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::string client_name = std::string(client_ip) + ":" + std::to_string(ntohs(client_addr.sin_port));

        std::thread client_thread(handle_client, client_socket, client_name);
        client_thread.detach(); // Detach the thread to run independently
    }

    close_socket(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    // The main function serves as a launcher and provides test instructions.
    std::cout << "--- C++ Server Test ---" << std::endl;
    std::cout << "How to Test:" << std::endl;
    std::cout << "1. Compile and run this program." << std::endl;
    std::cout << "   - On Linux/macOS: g++ -o server Task90.cpp -std=c++11 -pthread" << std::endl;
    std::cout << "   - On Windows (MinGW): g++ -o server.exe Task90.cpp -std=c++11 -pthread -lws2_32" << std::endl;
    std::cout << "2. Run the executable: ./server (or server.exe on Windows)" << std::endl;
    std::cout << "3. Open 5 separate terminal/command prompt windows." << std::endl;
    std::cout << "4. In each terminal, connect using telnet or netcat:" << std::endl;
    std::cout << "   telnet localhost 12345" << std::endl;
    std::cout << "5. Type messages and press Enter. They will be broadcasted." << std::endl;
    std::cout << "6. To quit a client, type 'exit' or close the terminal." << std::endl;
    std::cout << "7. To stop the server, use Ctrl+C in its terminal." << std::endl;
    std::cout << "--------------------------" << std::endl;
    
    run_server(12345);
    
    return 0;
}