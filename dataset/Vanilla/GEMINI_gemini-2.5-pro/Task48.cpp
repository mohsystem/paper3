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
    #define CLOSE_SOCKET(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define CLOSE_SOCKET(s) close(s)
    #define INVALID_SOCKET -1
#endif

const int PORT = 12345;
const char* HOST = "127.0.0.1";
const int BUFFER_SIZE = 1024;

std::vector<socket_t> clients;
std::mutex clients_mutex;
bool server_running = true;

void broadcast_message(const std::string& message, socket_t sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    std::cout << "Broadcasting: " << message;
    for (socket_t client_socket : clients) {
        if (client_socket != sender_socket) {
            send(client_socket, message.c_str(), message.length(), 0);
        }
    }
}

void handle_client(socket_t client_socket) {
    std::string client_name = "Client-" + std::to_string(client_socket);
    std::cout << client_name << " connected." << std::endl;
    std::string welcome_msg = client_name + " has joined the chat.\n";
    broadcast_message(welcome_msg, client_socket);

    char buffer[BUFFER_SIZE];
    int bytes_received;

    while ((bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[bytes_received] = '\0';
        std::string message = client_name + ": " + buffer + "\n";
        broadcast_message(message, client_socket);
    }

    std::cout << client_name << " disconnected." << std::endl;
    std::string leave_msg = client_name + " has left the chat.\n";
    
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    }
    
    broadcast_message(leave_msg, client_socket);
    CLOSE_SOCKET(client_socket);
}

void server_thread_func() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif

    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &server_addr.sin_addr);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        CLOSE_SOCKET(server_socket);
        return;
    }

    if (listen(server_socket, 5) < 0) {
        std::cerr << "Listen failed." << std::endl;
        CLOSE_SOCKET(server_socket);
        return;
    }

    std::cout << "Server started on port " << PORT << std::endl;

    while (server_running) {
        socket_t client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
             if (server_running) std::cerr << "Accept failed." << std::endl;
             continue;
        }

        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.push_back(client_socket);
        }

        std::thread(handle_client, client_socket).detach();
    }

    CLOSE_SOCKET(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
    std::cout << "Server stopped." << std::endl;
}

void client_thread_func(int id, const std::vector<std::string>& messages) {
    std::string client_name = "TestClient" + std::to_string(id);
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) { return; }
#endif
    
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << client_name << " socket creation error" << std::endl;
        return;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &serv_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << client_name << " connection Failed" << std::endl;
        CLOSE_SOCKET(sock);
        return;
    }

    // Listener thread
    std::thread([&]() {
        char buffer[BUFFER_SIZE];
        while (recv(sock, buffer, BUFFER_SIZE - 1, 0) > 0) {
            std::cout << client_name << " received: " << buffer << std::flush;
            memset(buffer, 0, BUFFER_SIZE);
        }
    }).detach();

    for (const auto& msg : messages) {
        send(sock, msg.c_str(), msg.length(), 0);
        std::this_thread::sleep_for(std::chrono::milliseconds(500 + (id * 100)));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    CLOSE_SOCKET(sock);
#ifdef _WIN32
    WSACleanup();
#endif
    std::cout << client_name << " finished." << std::endl;
}


int main() {
    std::thread server(server_thread_func);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // Test cases: 5 clients
    std::vector<std::thread> client_threads;
    client_threads.emplace_back(client_thread_func, 1, std::vector<std::string>{"Hello everyone!", "How are you?"});
    client_threads.emplace_back(client_thread_func, 2, std::vector<std::string>{"Hi!", "I am fine, thanks!"});
    client_threads.emplace_back(client_thread_func, 3, std::vector<std::string>{"Good day.", "Any news?"});
    client_threads.emplace_back(client_thread_func, 4, std::vector<std::string>{"Hey there!", "Just joined."});
    client_threads.emplace_back(client_thread_func, 5, std::vector<std::string>{"Greetings.", "Let's chat."});

    for (auto& t : client_threads) {
        t.join();
    }

    std::cout << "All clients finished. Stopping server..." << std::endl;
    server_running = false;
    // Create a dummy connection to unblock accept()
    socket_t dummy_socket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &server_addr.sin_addr);
    connect(dummy_socket, (struct sockaddr*)&server_addr, sizeof(server_addr));
    CLOSE_SOCKET(dummy_socket);

    server.join();

    std::cout << "Chat simulation finished." << std::endl;

    return 0;
}