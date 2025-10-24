#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <algorithm>
#include <cstring>
#include <chrono>

// Platform-specific socket includes and setup
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using socket_t = SOCKET;
    const socket_t INVALID_SOCK = INVALID_SOCKET;
    #define close_socket(s) closesocket(s)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    using socket_t = int;
    const socket_t INVALID_SOCK = -1;
    #define close_socket(s) close(s)
#endif

// Shared state for the server
std::vector<socket_t> clients;
std::mutex clients_mutex;
const int PORT = 12345;
const char* HOST = "127.0.0.1";
const int MAX_MSG_LENGTH = 512;

// RAII wrapper for network library initialization/cleanup
class NetworkInitializer {
public:
    NetworkInitializer() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed.\n";
            exit(1);
        }
#endif
    }
    ~NetworkInitializer() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
};

// Securely broadcasts a message to all clients
void broadcast_message(const std::string& message, socket_t sender_socket) {
    // Use std::lock_guard for exception-safe mutex locking (RAII).
    std::lock_guard<std::mutex> guard(clients_mutex);
    for (socket_t client_socket : clients) {
        // Broadcast to all, including sender for chat history
        if (send(client_socket, message.c_str(), message.length(), 0) < 0) {
            // Error handling: could log this, but for now we ignore.
            // The handle_client function will detect the disconnection.
        }
    }
}

// Handles a single client connection
void handle_client(socket_t client_socket) {
    char buffer[MAX_MSG_LENGTH + 1]; // +1 for null terminator

    // First message is the client's name
    int bytes_received = recv(client_socket, buffer, MAX_MSG_LENGTH, 0);
    std::string client_name;
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        client_name = buffer;
    } else {
        client_name = "Anonymous";
    }
    
    std::cout << client_name << " has connected." << std::endl;
    broadcast_message(client_name + " has joined the chat.", client_socket);

    {
        std::lock_guard<std::mutex> guard(clients_mutex);
        clients.push_back(client_socket);
    }

    // Main message loop
    while (true) {
        // Using recv with a fixed buffer size prevents buffer overflows.
        bytes_received = recv(client_socket, buffer, MAX_MSG_LENGTH, 0);
        if (bytes_received <= 0) { // 0 means clean disconnect, <0 is an error
            break;
        }
        buffer[bytes_received] = '\0'; // Ensure null termination
        std::string message(buffer);
        std::string full_message = client_name + ": " + message;
        broadcast_message(full_message, client_socket);
    }

    // Resource cleanup
    {
        std::lock_guard<std::mutex> guard(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    }
    broadcast_message(client_name + " has left the chat.", client_socket);
    std::cout << client_name << " has disconnected." << std::endl;
    close_socket(client_socket);
}

void run_server() {
    socket_t server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCK) {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        close_socket(server_fd);
        return;
    }

    if (listen(server_fd, 10) < 0) { // Backlog of 10
        std::cerr << "Listen failed" << std::endl;
        close_socket(server_fd);
        return;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        socket_t new_socket = accept(server_fd, nullptr, nullptr);
        if (new_socket == INVALID_SOCK) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }
        // Spawn a new thread for each client. Detach to let it run independently.
        std::thread(handle_client, new_socket).detach();
    }

    close_socket(server_fd);
}

void receive_messages(socket_t sock) {
    char buffer[MAX_MSG_LENGTH + 1];
    while (true) {
        int bytes_received = recv(sock, buffer, MAX_MSG_LENGTH, 0);
        if (bytes_received <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            break;
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer << std::endl;
    }
}

void run_client() {
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCK) {
        std::cerr << "Socket creation error" << std::endl;
        return;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, HOST, &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        close_socket(sock);
        return;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        close_socket(sock);
        return;
    }

    std::cout << "Enter your name: ";
    std::string name;
    std::getline(std::cin, name);
    send(sock, name.c_str(), name.length(), 0);

    // Thread for receiving messages
    std::thread receiver_thread(receive_messages, sock);
    receiver_thread.detach();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (!line.empty()) {
            send(sock, line.c_str(), line.length(), 0);
        }
    }
    close_socket(sock);
}

void run_test_client(const std::string& name, const std::string& message) {
    socket_t sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCK) return;

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close_socket(sock);
        return;
    }

    send(sock, name.c_str(), name.length(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    send(sock, message.c_str(), message.length(), 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    close_socket(sock);
}


void run_test_cases() {
    std::cout << "--- Running Automated Test Cases ---" << std::endl;
    // TC 1: Start Server
    std::thread server_thread(run_server);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // TC 2 & 3: Two clients connect and send messages
    std::thread client1(run_test_client, "ClientA", "Hello C++ World!");
    std::thread client2(run_test_client, "ClientB", "How are you?");

    std::this_thread::sleep_for(std::chrono::seconds(1));

    // TC 4: A third client connects
    std::thread client3(run_test_client, "ClientC", "I'm new here.");

    // TC 5: Wait for clients to finish
    client1.join();
    client2.join();
    client3.join();

    std::cout << "\n--- Test Cases Finished ---" << std::endl;
    // To stop the server, you would need a more complex signal mechanism.
    // For this example, we just exit, which terminates the detached threads.
    // The server_thread.detach() is needed to allow main to exit without joining.
    server_thread.detach();
}

int main(int argc, char const* argv[]) {
    NetworkInitializer network;
    if (argc > 1) {
        if (strcmp(argv[1], "server") == 0) {
            run_server();
        } else if (strcmp(argv[1], "client") == 0) {
            run_client();
        } else {
            std::cerr << "Usage: " << argv[0] << " [server|client]" << std::endl;
        }
    } else {
        run_test_cases();
        std::cout << "To run manually: " << argv[0] << " [server|client]" << std::endl;
    }
    return 0;
}