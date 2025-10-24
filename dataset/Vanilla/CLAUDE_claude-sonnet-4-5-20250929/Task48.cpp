
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define CLOSE_SOCKET close
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
#endif

class Task48 {
private:
    static const int PORT = 12345;
    static std::vector<class ClientHandler*> clients;
    static std::mutex clients_mutex;
    
public:
    class ClientHandler {
    private:
        socket_t socket;
        std::string clientName;
        
    public:
        ClientHandler(socket_t sock) : socket(sock) {}
        
        void handle() {
            char buffer[1024];
            
            send(socket, "Enter your name:\\n", 17, 0);
            int bytes = recv(socket, buffer, sizeof(buffer) - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = '\\0';
                clientName = std::string(buffer);
                clientName.erase(clientName.find_last_not_of("\\r\\n") + 1);
                Task48::broadcast(clientName + " has joined the chat!");
            }
            
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                bytes = recv(socket, buffer, sizeof(buffer) - 1, 0);
                if (bytes <= 0) break;
                
                buffer[bytes] = '\\0';
                std::string message(buffer);
                message.erase(message.find_last_not_of("\\r\\n") + 1);
                
                if (message == "/quit") break;
                
                Task48::broadcast(clientName + ": " + message);
            }
            
            cleanup();
        }
        
        void cleanup() {
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.erase(std::remove(clients.begin(), clients.end(), this), clients.end());
            }
            if (!clientName.empty()) {
                Task48::broadcast(clientName + " has left the chat!");
            }
            CLOSE_SOCKET(socket);
        }
        
        void sendMessage(const std::string& message) {
            std::string msg = message + "\\n";
            send(socket, msg.c_str(), msg.length(), 0);
        }
    };
    
    static void broadcast(const std::string& message) {
        std::cout << "[Server] " << message << std::endl;
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto client : clients) {
            client->sendMessage(message);
        }
    }
    
    static void startServer() {
        #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        #endif
        
        std::cout << "Chat server starting on port " << PORT << std::endl;
        
        socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) return;
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);
        
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            CLOSE_SOCKET(serverSocket);
            return;
        }
        
        listen(serverSocket, 5);
        
        while (true) {
            sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            socket_t clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket == INVALID_SOCKET) continue;
            
            ClientHandler* handler = new ClientHandler(clientSocket);
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.push_back(handler);
            }
            
            std::thread([handler]() {
                handler->handle();
                delete handler;
            }).detach();
        }
        
        CLOSE_SOCKET(serverSocket);
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
    
    class ChatClient {
    private:
        socket_t sock;
        bool running;
        
    public:
        ChatClient() : sock(INVALID_SOCKET), running(false) {}
        
        void connect(const std::string& host, int port, const std::string& name) {
            #ifdef _WIN32
            static bool wsaInitialized = false;
            if (!wsaInitialized) {
                WSADATA wsaData;
                WSAStartup(MAKEWORD(2, 2), &wsaData);
                wsaInitialized = true;
            }
            #endif
            
            sock = socket(AF_INET, SOCK_STREAM, 0);
            
            sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(port);
            inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
            
            ::connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr));
            running = true;
            
            char buffer[1024];
            recv(sock, buffer, sizeof(buffer), 0);
            std::cout << buffer;
            
            std::string nameMsg = name + "\\n";
            send(sock, nameMsg.c_str(), nameMsg.length(), 0);
            
            std::thread([this]() {
                char buffer[1024];
                while (running) {
                    memset(buffer, 0, sizeof(buffer));
                    int bytes = recv(sock, buffer, sizeof(buffer) - 1, 0);
                    if (bytes > 0) {
                        std::cout << buffer;
                    } else {
                        break;
                    }
                }
            }).detach();
        }
        
        void sendMessage(const std::string& message) {
            std::string msg = message + "\\n";
            send(sock, msg.c_str(), msg.length(), 0);
        }
        
        void disconnect() {
            running = false;
            if (sock != INVALID_SOCKET) {
                CLOSE_SOCKET(sock);
            }
        }
    };
};

std::vector<Task48::ClientHandler*> Task48::clients;
std::mutex Task48::clients_mutex;

int main() {
    std::thread serverThread(Task48::startServer);
    serverThread.detach();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    try {
        std::cout << "\\n=== Test Case 1: Single client connection ===" << std::endl;
        Task48::ChatClient client1;
        client1.connect("127.0.0.1", 12345, "Alice");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        client1.sendMessage("Hello from Alice!");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "\\n=== Test Case 2: Multiple clients ===" << std::endl;
        Task48::ChatClient client2;
        client2.connect("127.0.0.1", 12345, "Bob");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        client2.sendMessage("Hi everyone from Bob!");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "\\n=== Test Case 3: Broadcasting ===" << std::endl;
        client1.sendMessage("This is Alice speaking");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        client2.sendMessage("Bob here too");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "\\n=== Test Case 4: Third client joins ===" << std::endl;
        Task48::ChatClient client3;
        client3.connect("127.0.0.1", 12345, "Charlie");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        client3.sendMessage("Charlie joined the chat");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "\\n=== Test Case 5: Client disconnection ===" << std::endl;
        client1.sendMessage("/quit");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        client2.sendMessage("Alice left");
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        client2.disconnect();
        client3.disconnect();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
    } catch (...) {
        std::cerr << "Error occurred" << std::endl;
    }
    
    return 0;
}
