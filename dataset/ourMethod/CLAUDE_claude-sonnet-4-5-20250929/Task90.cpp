
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <csignal>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define CLOSE_SOCKET close
#endif

// Maximum limits to prevent resource exhaustion
constexpr size_t MAX_MESSAGE_SIZE = 4096;
constexpr size_t MAX_CLIENTS = 100;
constexpr int MAX_RECV_ATTEMPTS = 3;

std::mutex clients_mutex;
std::vector<SOCKET> clients;
std::atomic<bool> server_running(true);

// Secure message broadcasting with bounds checking and validation
void broadcast_message(const std::string& message, SOCKET sender_socket) {
    // Validate message size before broadcasting
    if (message.empty() || message.size() > MAX_MESSAGE_SIZE) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(clients_mutex);
    
    // Create a safe buffer with size validation
    std::vector<char> buffer(message.begin(), message.end());
    
    // Iterate safely with size bounds
    for (size_t i = 0; i < clients.size(); ) {
        if (clients[i] != sender_socket) {
            // Send with error checking - send returns actual bytes sent
            ssize_t sent = send(clients[i], buffer.data(), static_cast<int>(buffer.size()), 0);
            if (sent == SOCKET_ERROR) {
                // Remove disconnected client safely
                CLOSE_SOCKET(clients[i]);
                clients.erase(clients.begin() + i);
                continue;
            }
        }
        ++i;
    }
}

// Secure client handler with proper resource cleanup and input validation
void handle_client(SOCKET client_socket) {
    // RAII ensures socket cleanup on function exit
    struct SocketGuard {
        SOCKET sock;
        explicit SocketGuard(SOCKET s) : sock(s) {}
        ~SocketGuard() { 
            if (sock != INVALID_SOCKET) {
                CLOSE_SOCKET(sock); 
            }
        }
    } guard(client_socket);
    
    // Fixed-size buffer to prevent unbounded memory growth
    std::vector<char> buffer(MAX_MESSAGE_SIZE + 1, 0);
    
    while (server_running.load()) {
        // Clear buffer before each receive to prevent data leakage
        std::fill(buffer.begin(), buffer.end(), 0);
        
        // Receive with bounds checking - never exceed buffer size
        ssize_t bytes_received = recv(client_socket, buffer.data(), MAX_MESSAGE_SIZE, 0);
        
        // Validate receive operation
        if (bytes_received <= 0) {
            break; // Client disconnected or error
        }
        
        // Ensure null termination within bounds
        if (static_cast<size_t>(bytes_received) < buffer.size()) {
            buffer[bytes_received] = '\\0';
        } else {
            buffer[MAX_MESSAGE_SIZE] = '\\0';
        }
        
        // Validate received data contains only printable characters
        bool valid_input = true;
        for (ssize_t i = 0; i < bytes_received; ++i) {
            if (buffer[i] != '\\n' && buffer[i] != '\\r' && 
                (buffer[i] < 32 || buffer[i] > 126)) {
                valid_input = false;
                break;
            }
        }
        
        if (!valid_input) {
            continue; // Skip invalid messages
        }
        
        // Create validated message string with size limit
        std::string message(buffer.data(), std::min(static_cast<size_t>(bytes_received), MAX_MESSAGE_SIZE));
        
        // Broadcast validated message
        broadcast_message(message, client_socket);
    }
    
    // Remove client from active list with thread-safe operations
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        auto it = std::find(clients.begin(), clients.end(), client_socket);
        if (it != clients.end()) {
            clients.erase(it);
        }
    }
}

// Secure server initialization and operation
int run_server(int port) {
    // Validate port range to prevent invalid configurations
    if (port < 1024 || port > 65535) {
        std::cerr << "Port must be between 1024 and 65535" << std::endl;
        return 1;
    }
    
#ifdef _WIN32
    WSADATA wsa_data;
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif
    
    // Create socket with error checking
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed" << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    // RAII for socket cleanup
    struct ServerSocketGuard {
        SOCKET sock;
        explicit ServerSocketGuard(SOCKET s) : sock(s) {}
        ~ServerSocketGuard() { 
            if (sock != INVALID_SOCKET) {
                CLOSE_SOCKET(sock);
            }
#ifdef _WIN32
            WSACleanup();
#endif
        }
    } socket_guard(server_socket);
    
    // Enable address reuse to prevent "address already in use" errors
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, 
                   reinterpret_cast<const char*>(&opt), sizeof(opt)) == SOCKET_ERROR) {
        std::cerr << "Setsockopt failed" << std::endl;
        return 1;
    }
    
    // Initialize server address structure with secure defaults
    sockaddr_in server_addr;
    std::memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(static_cast<uint16_t>(port));
    
    // Bind with error checking
    if (bind(server_socket, reinterpret_cast<sockaddr*>(&server_addr), 
             sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }
    
    // Listen with reasonable backlog
    if (listen(server_socket, 10) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }
    
    std::cout << "Server listening on port " << port << std::endl;
    
    std::vector<std::thread> threads;
    
    while (server_running.load()) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        std::memset(&client_addr, 0, sizeof(client_addr));
        
        // Accept with validation
        SOCKET client_socket = accept(server_socket, 
                                     reinterpret_cast<sockaddr*>(&client_addr), 
                                     &client_len);
        
        if (client_socket == INVALID_SOCKET) {
            continue;
        }
        
        // Enforce maximum client limit to prevent resource exhaustion
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            if (clients.size() >= MAX_CLIENTS) {
                std::cerr << "Maximum clients reached, rejecting connection" << std::endl;
                CLOSE_SOCKET(client_socket);
                continue;
            }
            clients.push_back(client_socket);
        }
        
        // Spawn thread to handle client with exception safety
        try {
            threads.emplace_back(handle_client, client_socket);
        } catch (const std::exception& e) {
            std::cerr << "Failed to create thread: " << e.what() << std::endl;
            CLOSE_SOCKET(client_socket);
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.pop_back();
        }
    }
    
    // Clean shutdown - join all threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    return 0;
}

int main() {
    // Test cases with validated ports
    std::vector<int> test_ports = {8080, 8081, 8082, 8083, 8084};
    
    std::cout << "Testing chat server with port: " << test_ports[0] << std::endl;
    
    // Run server on first test port
    return run_server(test_ports[0]);
}
