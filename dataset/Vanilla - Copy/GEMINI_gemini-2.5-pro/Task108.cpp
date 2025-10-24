#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <vector>

// For networking - POSIX compliant (Linux, macOS)
// For Windows, you would need to include <winsock2.h> and link against ws2_32.lib
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

const int PORT = 8888;
const int BUFFER_SIZE = 1024;

// --- Server Logic ---
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE);
    
    std::stringstream ss(buffer);
    std::string command;
    double a, b, result;
    ss >> command >> a >> b;

    std::string response;
    try {
        if (command == "ADD") {
            result = a + b;
        } else if (command == "SUB") {
            result = a - b;
        } else if (command == "MUL") {
            result = a * b;
        } else if (command == "DIV") {
            if (b == 0) {
                throw std::runtime_error("Error: Division by zero.");
            }
            result = a / b;
        } else {
            throw std::runtime_error("Error: Unknown command.");
        }
        response = std::to_string(result);
    } catch (const std::exception& e) {
        response = e.what();
    }
    
    send(client_socket, response.c_str(), response.length(), 0);
    close(client_socket);
}

void run_server() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server listening on port " << PORT << std::endl;
    
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue listening
        }
        handle_client(new_socket);
    }
}

// --- Client Logic ---
std::string send_request(const std::string& request) {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return "Socket creation error";
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        return "Invalid address/ Address not supported";
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        return "Connection Failed. Is the server running?";
    }

    send(sock, request.c_str(), request.length(), 0);
    read(sock, buffer, BUFFER_SIZE);
    close(sock);
    return std::string(buffer);
}

void run_client() {
    std::cout << "Client connected to server." << std::endl;
    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Addition
    std::cout << "Test 1: 5 + 3 = " << send_request("ADD 5 3") << std::endl;
    
    // Test Case 2: Subtraction
    std::cout << "Test 2: 10 - 4 = " << send_request("SUB 10 4") << std::endl;

    // Test Case 3: Multiplication
    std::cout << "Test 3: 7 * 6 = " << send_request("MUL 7 6") << std::endl;
    
    // Test Case 4: Division
    std::cout << "Test 4: 20 / 5 = " << send_request("DIV 20 5") << std::endl;

    // Test Case 5: Division by zero
    std::cout << "Test 5: 10 / 0 -> " << send_request("DIV 10 0") << std::endl;
}

/**
 * Main function to launch server or client.
 * NOTE: This code uses POSIX sockets. For Windows, use Winsock.
 * --- How to Run ---
 * 1. Compile: g++ -o Task108 Task108.cpp
 * 2. Start Server in one terminal: ./Task108 server
 * 3. Start Client in another terminal: ./Task108 client
 */
class Task108 {
public:
    static void main(int argc, char* argv[]) {
        if (argc < 2) {
            std::cerr << "Usage: " << argv[0] << " <server|client>" << std::endl;
            exit(EXIT_FAILURE);
        }

        std::string mode = argv[1];
        if (mode == "server") {
            run_server();
        } else if (mode == "client") {
            run_client();
        } else {
            std::cerr << "Invalid mode. Use 'server' or 'client'." << std::endl;
            exit(EXIT_FAILURE);
        }
    }
};

int main(int argc, char* argv[]) {
    Task108::main(argc, argv);
    return 0;
}