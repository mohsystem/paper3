
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>

// Security: Max message size to prevent buffer overflow attacks
const size_t MAX_MESSAGE_SIZE = 4096;
// Security: Max clients to prevent resource exhaustion
const size_t MAX_CLIENTS = 100;
// Security: Port validation range
const int MIN_PORT = 1024;
const int MAX_PORT = 65535;

class ChatServer {
private:
    int server_fd;
    std::vector<int> client_fds;
    std::mutex clients_mutex;
    std::atomic<bool> running;

    // Security: Validate message content and size before broadcasting
    bool validateMessage(const char* msg, size_t len) {
        // Security: Check message length to prevent overflow
        if (len == 0 || len > MAX_MESSAGE_SIZE) {
            return false;
        }
        // Security: Ensure null-terminated string
        if (msg[len - 1] != '\\0') {
            return false;
        }
        return true;
    }

    void broadcastMessage(const std::string& message, int sender_fd) {
        std::lock_guard<std::mutex> lock(clients_mutex);
        
        // Security: Validate message size before sending
        if (message.size() > MAX_MESSAGE_SIZE - 1) {
            return;
        }

        for (int client_fd : client_fds) {
            if (client_fd != sender_fd && client_fd >= 0) {
                // Security: Use send with MSG_NOSIGNAL to prevent SIGPIPE
                ssize_t sent = send(client_fd, message.c_str(), message.size(), MSG_NOSIGNAL);
                // Security: Check return value for errors
                if (sent < 0) {
                    std::cerr << "Failed to send to client" << std::endl;
                }
            }
        }
    }

    void handleClient(int client_fd) {
        char buffer[MAX_MESSAGE_SIZE];
        
        while (running.load()) {
            // Security: Clear buffer before reading to prevent data leakage
            std::memset(buffer, 0, sizeof(buffer));
            
            // Security: Use recv with size limit to prevent buffer overflow
            ssize_t bytes_read = recv(client_fd, buffer, MAX_MESSAGE_SIZE - 1, 0);
            
            // Security: Check return value for errors or connection close
            if (bytes_read <= 0) {
                break;
            }
            
            // Security: Ensure null-termination
            buffer[bytes_read] = '\\0';
            
            // Security: Validate message before broadcasting
            if (validateMessage(buffer, bytes_read + 1)) {
                std::string message = "Client " + std::to_string(client_fd) + ": " + std::string(buffer);
                broadcastMessage(message, client_fd);
            }
        }
        
        // Security: Remove client and close socket
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            client_fds.erase(std::remove(client_fds.begin(), client_fds.end(), client_fd), client_fds.end());
        }
        close(client_fd);
    }

public:
    ChatServer() : server_fd(-1), running(false) {}

    ~ChatServer() {
        stop();
    }

    // Security: Validate port range
    bool start(int port) {
        if (port < MIN_PORT || port > MAX_PORT) {
            std::cerr << "Invalid port range" << std::endl;
            return false;
        }

        // Security: Create socket with error checking
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) {
            std::cerr << "Socket creation failed" << std::endl;
            return false;
        }

        // Security: Set socket options to reuse address
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            close(server_fd);
            return false;
        }

        struct sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Security: Check bind return value
        if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            close(server_fd);
            return false;
        }

        // Security: Check listen return value
        if (listen(server_fd, 10) < 0) {
            std::cerr << "Listen failed" << std::endl;
            close(server_fd);
            return false;
        }

        running.store(true);
        std::cout << "Server started on port " << port << std::endl;

        while (running.load()) {
            struct pollfd pfd;
            pfd.fd = server_fd;
            pfd.events = POLLIN;
            
            // Security: Use poll with timeout to allow graceful shutdown
            int ret = poll(&pfd, 1, 1000);
            if (ret < 0) {
                break;
            }
            if (ret == 0) {
                continue;
            }

            struct sockaddr_in client_addr;
            socklen_t client_len = sizeof(client_addr);
            
            // Security: Accept with error checking
            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
            if (client_fd < 0) {
                continue;
            }

            // Security: Check max clients limit to prevent resource exhaustion
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                if (client_fds.size() >= MAX_CLIENTS) {
                    close(client_fd);
                    continue;
                }
                client_fds.push_back(client_fd);
            }

            // Security: Launch thread to handle client
            std::thread(&ChatServer::handleClient, this, client_fd).detach();
        }

        return true;
    }

    void stop() {
        running.store(false);
        
        // Security: Close all client connections
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            for (int fd : client_fds) {
                if (fd >= 0) {
                    close(fd);
                }
            }
            client_fds.clear();
        }
        
        // Security: Close server socket
        if (server_fd >= 0) {
            close(server_fd);
            server_fd = -1;
        }
    }
};

class ChatClient {
private:
    int sock_fd;
    std::atomic<bool> running;

    void receiveMessages() {
        char buffer[MAX_MESSAGE_SIZE];
        
        while (running.load()) {
            // Security: Clear buffer before reading
            std::memset(buffer, 0, sizeof(buffer));
            
            // Security: Use recv with size limit
            ssize_t bytes_read = recv(sock_fd, buffer, MAX_MESSAGE_SIZE - 1, 0);
            
            // Security: Check return value
            if (bytes_read <= 0) {
                break;
            }
            
            // Security: Ensure null-termination
            buffer[bytes_read] = '\\0';
            std::cout << buffer << std::endl;
        }
    }

public:
    ChatClient() : sock_fd(-1), running(false) {}

    ~ChatClient() {
        disconnect();
    }

    // Security: Validate IP address and port
    bool connect(const std::string& ip, int port) {
        if (port < MIN_PORT || port > MAX_PORT) {
            return false;
        }

        // Security: Create socket with error checking
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            return false;
        }

        struct sockaddr_in serv_addr;
        std::memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        // Security: Validate IP address conversion
        if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
            close(sock_fd);
            return false;
        }

        // Security: Check connect return value
        if (::connect(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
            close(sock_fd);
            return false;
        }

        running.store(true);
        std::thread(&ChatClient::receiveMessages, this).detach();
        return true;
    }

    // Security: Validate message size before sending
    bool sendMessage(const std::string& message) {
        if (message.empty() || message.size() > MAX_MESSAGE_SIZE - 1) {
            return false;
        }

        // Security: Use send with MSG_NOSIGNAL
        ssize_t sent = send(sock_fd, message.c_str(), message.size(), MSG_NOSIGNAL);
        return sent > 0;
    }

    void disconnect() {
        running.store(false);
        if (sock_fd >= 0) {
            close(sock_fd);
            sock_fd = -1;
        }
    }
};

int main() {
    std::cout << "=== Test Case 1: Server Start ===" << std::endl;
    ChatServer server;
    std::thread server_thread([&server]() {
        server.start(8080);
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "=== Test Case 2: Client Connection ===" << std::endl;
    ChatClient client1;
    if (client1.connect("127.0.0.1", 8080)) {
        std::cout << "Client 1 connected" << std::endl;
    }

    std::cout << "=== Test Case 3: Send Message ===" << std::endl;
    client1.sendMessage("Hello from client 1");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "=== Test Case 4: Multiple Clients ===" << std::endl;
    ChatClient client2;
    if (client2.connect("127.0.0.1", 8080)) {
        std::cout << "Client 2 connected" << std::endl;
    }
    client2.sendMessage("Hello from client 2");
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::cout << "=== Test Case 5: Cleanup ===" << std::endl;
    client1.disconnect();
    client2.disconnect();
    server.stop();
    
    if (server_thread.joinable()) {
        server_thread.join();
    }
    
    std::cout << "All tests completed" << std::endl;
    return 0;
}
