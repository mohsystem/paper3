#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <fstream>
#include <sstream>
#include <map>
#include <cstring>
#include <atomic>

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

const int PORT = 12347;
const char* HOST = "127.0.0.1";
const char* USER_FILE = "users_cpp.txt";
std::atomic<bool> server_should_stop(false);

// --- SERVER COMPONENT ---

class Server {
private:
    std::map<std::string, std::string> users;

    void loadUsers() {
        std::ifstream file(USER_FILE);
        std::string line;
        if (!file.is_open()) {
            std::cerr << "Server Error: Could not open user file." << std::endl;
            return;
        }
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string username, password;
            if (std::getline(ss, username, ':') && std::getline(ss, password)) {
                users[username] = password;
            }
        }
        std::cout << "Users loaded from " << USER_FILE << std::endl;
        file.close();
    }

    std::string authenticate(const std::string& username, const std::string& password) {
        auto it = users.find(username);
        if (it == users.end()) {
            return "LOGIN_FAIL User not found";
        }
        if (it->second == password) {
            return "LOGIN_SUCCESS";
        } else {
            return "LOGIN_FAIL Invalid password";
        }
    }

    void handle_client(SOCKET clientSocket) {
        char buffer[1024] = {0};
        while (true) {
            int valread = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            if (valread <= 0) {
                break;
            }
            buffer[valread] = '\0';
            std::string received(buffer);
            received.erase(received.find_last_not_of("\r\n") + 1); // Trim newline
            std::cout << "Server received: " << received << std::endl;

            std::stringstream ss(received);
            std::string command, username, password;
            ss >> command >> username >> password;

            std::string response;
            if (command == "LOGIN" && !username.empty() && !password.empty()) {
                response = authenticate(username, password);
            } else if (command == "SHUTDOWN") {
                server_should_stop = true;
                break;
            }
            else {
                response = "ERROR Unknown command";
            }
            response += "\n";
            send(clientSocket, response.c_str(), response.length(), 0);
        }
        closesocket(clientSocket);
    }


public:
    void start() {
        #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return;
        }
        #endif

        loadUsers();
        SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }
        
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(PORT);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            closesocket(server_fd);
            return;
        }

        if (listen(server_fd, 3) < 0) {
            std::cerr << "Listen failed" << std::endl;
            closesocket(server_fd);
            return;
        }

        std::cout << "Server started on port " << PORT << std::endl;

        while (!server_should_stop) {
            SOCKET new_socket = accept(server_fd, nullptr, nullptr);
            if (new_socket == INVALID_SOCKET) {
                if(server_should_stop) break;
                std::cerr << "Accept failed" << std::endl;
                continue;
            }
            // For simplicity, handle one client at a time in a blocking way.
            // A real server would use threads.
            handle_client(new_socket);
        }

        closesocket(server_fd);
        #ifdef _WIN32
        WSACleanup();
        #endif
        std::cout << "Server stopped." << std::endl;
    }
};

// --- CLIENT COMPONENT ---

void run_client() {
    std::cout << "\n--- Starting Client Test Cases ---" << std::endl;
    
    #ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
    #endif

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Client: Socket creation error" << std::endl;
        return;
    }

    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, HOST, &serv_addr.sin_addr);
    
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Client: Connection Failed" << std::endl;
        closesocket(sock);
        return;
    }

    auto send_and_receive = [&](const std::string& request) {
        std::string full_request = request + "\n";
        std::cout << "Client sent: " << request << std::endl;
        send(sock, full_request.c_str(), full_request.length(), 0);
        char buffer[1024] = {0};
        recv(sock, buffer, 1024, 0);
        std::string response(buffer);
        response.erase(response.find_last_not_of("\r\n") + 1);
        std::cout << "Server response: " << response << std::endl;
    };
    
    std::cout << "Client> Test 1: Correct credentials (alice)" << std::endl;
    send_and_receive("LOGIN alice pass123");

    std::cout << "\nClient> Test 2: Incorrect password (bob)" << std::endl;
    send_and_receive("LOGIN bob wrongpass");
    
    std::cout << "\nClient> Test 3: User not found (dave)" << std::endl;
    send_and_receive("LOGIN dave somepass");

    std::cout << "\nClient> Test 4: Correct credentials (charlie)" << std::endl;
    send_and_receive("LOGIN charlie qwerty");
    
    std::cout << "\nClient> Test 5: Malformed request" << std::endl;
    send_and_receive("LOGI charlie qwerty");

    // Signal server to shut down
    std::string shutdown_msg = "SHUTDOWN\n";
    send(sock, shutdown_msg.c_str(), shutdown_msg.length(), 0);

    closesocket(sock);
    #ifdef _WIN32
    WSACleanup();
    #endif
    std::cout << "--- Client Test Cases Finished ---" << std::endl;
}


int main() {
    // 1. Create user file
    std::ofstream user_file(USER_FILE);
    if (user_file.is_open()) {
        user_file << "alice:pass123\n";
        user_file << "bob:secret\n";
        user_file << "charlie:qwerty\n";
        user_file.close();
    }

    // 2. Start server in a background thread
    Server server;
    std::thread server_thread([&]() { server.start(); });
    
    // 3. Wait for server to start
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // 4. Run client test cases
    run_client();

    // 5. Wait for server thread to complete
    server_thread.join();

    // 6. Clean up
    remove(USER_FILE);
    std::cout << "\nProgram finished." << std::endl;
    
    return 0;
}