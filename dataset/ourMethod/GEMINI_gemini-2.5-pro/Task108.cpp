#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <sstream>
#include <cstring>
#include <atomic>
#include <cctype>

// Platform-specific socket includes
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

// Shared state for the remote object
static std::unordered_map<std::string, std::string> remote_object;
static std::mutex remote_object_mutex;

// A simple, thread-safe remote object server.
// NOTE: This implementation does not use TLS/SSL for encrypted communication
// or any form of authentication. In a production environment, these are critical.

std::string process_command(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> parts;
    std::string part;
    iss >> part;
    parts.push_back(part);
    if (iss) { // If there's more in the stream
        std::string rest;
        std::getline(iss, rest);
        if (!rest.empty() && rest[0] == ' ') {
            rest = rest.substr(1);
        }
        std::istringstream rest_ss(rest);
        while(rest_ss >> part) parts.push_back(part);
    }
    
    if (parts.empty() || parts[0].empty()) return "";

    // Rule #4: Validate and sanitize all input.
    std::string command = parts[0];
    for(char &c : command) c = std::toupper(c);

    const size_t MAX_KEY_LENGTH = 256;
    const size_t MAX_VALUE_LENGTH = 1024;

    try {
        if (command == "PUT") {
            if (parts.size() < 3) return "ERROR: PUT requires 2 arguments: key and value.\n";
            const std::string& key = parts[1];
            std::string value;
            size_t val_start = line.find(key) + key.length() + 1;
            if (val_start < line.length()) value = line.substr(val_start);
            
            if (key.length() > MAX_KEY_LENGTH || value.length() > MAX_VALUE_LENGTH) {
                return "ERROR: Key or value exceeds maximum length.\n";
            }
            // Rule #1: Use locks to prevent race conditions (TOCTOU)
            std::lock_guard<std::mutex> guard(remote_object_mutex);
            remote_object[key] = value;
            return "OK\n";
        } else if (command == "GET") {
            if (parts.size() != 2) return "ERROR: GET requires 1 argument: key.\n";
            const std::string& key = parts[1];
            if (key.length() > MAX_KEY_LENGTH) return "ERROR: Key exceeds maximum length.\n";
            std::lock_guard<std::mutex> guard(remote_object_mutex);
            auto it = remote_object.find(key);
            if (it != remote_object.end()) {
                return "VALUE " + it->second + "\n";
            } else {
                return "NOT_FOUND\n";
            }
        } else if (command == "DELETE") {
            if (parts.size() != 2) return "ERROR: DELETE requires 1 argument: key.\n";
            const std::string& key = parts[1];
            if (key.length() > MAX_KEY_LENGTH) return "ERROR: Key exceeds maximum length.\n";
            std::lock_guard<std::mutex> guard(remote_object_mutex);
            if (remote_object.erase(key)) {
                return "OK\n";
            } else {
                return "NOT_FOUND\n";
            }
        } else if (command == "EXIT") {
            return "GOODBYE\n";
        } else {
            // Rule #4: Reject unexpected commands.
            return "ERROR: Unknown command '" + parts[0] + "'.\n";
        }
    } catch (...) {
        // Rule #14: Ensure exceptions are caught and handled.
        return "ERROR: An internal server error occurred.\n";
    }
}

void handle_client(SOCKET client_socket) {
    char buffer[4096];
    std::string current_line;

    while (true) {
        // Rule #6: Use boundary checks to prevent overflows.
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);

        if (bytes_received <= 0) break;

        current_line.append(buffer, bytes_received);

        size_t pos;
        while ((pos = current_line.find('\n')) != std::string::npos) {
            std::string command_line = current_line.substr(0, pos);
            current_line.erase(0, pos + 1);

            if (!command_line.empty() && command_line.back() == '\r') {
                command_line.pop_back();
            }

            std::string response = process_command(command_line);
            send(client_socket, response.c_str(), response.length(), 0);

            if (response == "GOODBYE\n") {
                closesocket(client_socket);
                return;
            }
        }
    }
    closesocket(client_socket);
}

std::atomic<bool> server_running(true);

void server_main(const char* host, int port) {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed.\n";
        return;
    }
#endif
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) { std::cerr << "Socket creation failed.\n"; return; }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &server_addr.sin_addr);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n"; closesocket(server_socket); return;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n"; closesocket(server_socket); return;
    }
    std::cout << "Server listening on port " << port << std::endl;

    while (server_running) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
             if (server_running) std::cerr << "Accept failed.\n";
            continue;
        }
        std::thread(handle_client, client_socket).detach();
    }

    closesocket(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
    std::cout << "Server stopped." << std::endl;
}

std::string send_request(const char* host, int port, const std::string& request) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return "CLIENT_ERROR: Socket creation failed.";

    sockaddr_in serv_addr{};
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if(inet_pton(AF_INET, host, &serv_addr.sin_addr) <= 0) {
        closesocket(sock);
        return "CLIENT_ERROR: Invalid address.";
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        closesocket(sock);
        return "CLIENT_ERROR: Connection Failed.";
    }

    std::string req = request + "\n";
    send(sock, req.c_str(), req.length(), 0);
    
    char buffer[4096] = {0};
    recv(sock, buffer, sizeof(buffer) - 1, 0);
    closesocket(sock);

    std::string response(buffer);
    size_t last_char = response.find_last_not_of("\r\n");
    if (last_char != std::string::npos) response.erase(last_char + 1);

    return response;
}

int main() {
    const int port = 12347;
    const char* host = "127.0.0.1";

    std::thread server_thread(server_main, host, port);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    std::cout << "--- Running Test Cases ---" << std::endl;

    std::cout << "Test Case 1: PUT and GET" << std::endl;
    std::string res1 = send_request(host, port, "PUT key1 value with spaces");
    std::cout << "Client sent: PUT key1 value with spaces -> Server responded: " << res1 << std::endl;
    std::string res2 = send_request(host, port, "GET key1");
    std::cout << "Client sent: GET key1 -> Server responded: " << res2 << std::endl << std::endl;

    std::cout << "Test Case 2: GET non-existent key" << std::endl;
    std::string res3 = send_request(host, port, "GET non_existent_key");
    std::cout << "Client sent: GET non_existent_key -> Server responded: " << res3 << std::endl << std::endl;

    std::cout << "Test Case 3: DELETE a key" << std::endl;
    std::string res4 = send_request(host, port, "DELETE key1");
    std::cout << "Client sent: DELETE key1 -> Server responded: " << res4 << std::endl;
    std::string res5 = send_request(host, port, "GET key1");
    std::cout << "Client sent: GET key1 -> Server responded: " << res5 << std::endl << std::endl;

    std::cout << "Test Case 4: Invalid command" << std::endl;
    std::string res6 = send_request(host, port, "INVALID_COMMAND");
    std::cout << "Client sent: INVALID_COMMAND -> Server responded: " << res6 << std::endl << std::endl;

    std::cout << "Test Case 5: Malformed command" << std::endl;
    std::string res7 = send_request(host, port, "PUT key2");
    std::cout << "Client sent: PUT key2 -> Server responded: " << res7 << std::endl << std::endl;

    server_running = false;
    send_request(host, port, "EXIT");

    if (server_thread.joinable()) server_thread.join();
    
    std::cout << "--- Test Cases Finished ---" << std::endl;
    return 0;
}