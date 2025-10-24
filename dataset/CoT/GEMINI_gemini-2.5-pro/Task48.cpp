#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef SOCKET socket_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    typedef int socket_t;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

const int PORT = 12345;
const char* HOST = "127.0.0.1";
const int BUFFER_SIZE = 4096;

std::vector<socket_t> clients;
std::mutex clients_mutex;

void broadcast_message(const std::string& message, socket_t sender_socket) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (socket_t client_socket : clients) {
        if (client_socket != sender_socket) {
            send(client_socket, message.c_str(), message.length(), 0);
        }
    }
}

void handle_client(socket_t client_socket) {
    char buffer[BUFFER_SIZE];
    
    // Get client name
    std::string name_prompt = "Enter your name: ";
    send(client_socket, name_prompt.c_str(), name_prompt.length(), 0);
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received <= 0) {
        closesocket(client_socket);
        return;
    }
    buffer[bytes_received] = '\0';
    std::string client_name(buffer);
    // Remove trailing newline characters
    client_name.erase(client_name.find_last_not_of("\r\n") + 1);

    std::cout << client_name << " has connected." << std::endl;

    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.push_back(client_socket);
    }

    std::string welcome_message = client_name + " has joined the chat.";
    broadcast_message(welcome_message, client_socket);

    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        if (bytes_received <= 0 || strncmp(buffer, "exit", 4) == 0) {
            break;
        }
        
        buffer[bytes_received] = '\0';
        std::string message(buffer);
        message.erase(message.find_last_not_of("\r\n") + 1);

        std::string broadcast_msg = client_name + ": " + message;
        std::cout << "Broadcasting: " << broadcast_msg << std::endl;
        broadcast_message(broadcast_msg, client_socket);
    }

    // Cleanup
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (*it == client_socket) {
                clients.erase(it);
                break;
            }
        }
    }
    
    std::string leave_message = client_name + " has left the chat.";
    std::cout << client_name << " has disconnected." << std::endl;
    broadcast_message(leave_message, client_socket);
    closesocket(client_socket);
}

void start_server() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif

    socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(server_socket);
        return;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(server_socket);
        return;
    }

    std::cout << "Server started on port " << PORT << ". Waiting for connections..." << std::endl;

    while (true) {
        socket_t client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }
        std::thread(handle_client, client_socket).detach();
    }

    closesocket(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

void receive_messages(socket_t sock) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            std::cout << "Disconnected from server." << std::endl;
            closesocket(sock);
            exit(0);
        }
        buffer[bytes_received] = '\0';
        std::cout << buffer << std::endl;
    }
}

void start_client() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif

    socket_t client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed. Is the server running?" << std::endl;
        closesocket(client_socket);
        return;
    }

    std::cout << "Connected to the chat server. Type 'exit' to quit." << std::endl;

    char buffer[BUFFER_SIZE];
    int bytes_received = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::cout << buffer;
        std::string name;
        std::getline(std::cin, name);
        send(client_socket, name.c_str(), name.length(), 0);
    }
    
    std::thread(receive_messages, client_socket).detach();

    std::string line;
    while (std::getline(std::cin, line)) {
        if (line == "exit") {
            send(client_socket, line.c_str(), line.length(), 0);
            break;
        }
        send(client_socket, line.c_str(), line.length(), 0);
    }
    
    closesocket(client_socket);
#ifdef _WIN32
    WSACleanup();
#endif
}

void print_usage() {
    std::cout << "Usage: ./program_name <mode>" << std::endl;
    std::cout << "  mode: 'server' or 'client'" << std::endl;
    std::cout << "\n--- How to Test (5 Test Cases) ---" << std::endl;
    std::cout << "1. Compile the code: g++ your_file.cpp -o program_name -std=c++11 -pthread (on Windows, add -lws2_32)" << std::endl;
    std::cout << "2. Start the server: Open a terminal and run './program_name server'" << std::endl;
    std::cout << "3. Start Client 1: Open a second terminal and run './program_name client'. Enter a name." << std::endl;
    std::cout << "4. Start Client 2: Open a third terminal and run './program_name client'. Enter another name." << std::endl;
    std::cout << "5. Send Messages & Disconnect: Type messages to chat. Type 'exit' to disconnect a client." << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }

    std::string mode = argv[1];
    if (mode == "server") {
        start_server();
    } else if (mode == "client") {
        start_client();
    } else {
        print_usage();
    }

    return 0;
}