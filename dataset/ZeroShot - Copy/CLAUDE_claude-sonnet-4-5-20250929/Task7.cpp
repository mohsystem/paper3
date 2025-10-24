
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <thread>
#include <chrono>
#include <random>
#include <iomanip>

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

#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

class Task7 {
public:
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;
        
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, buffer, length);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);
        
        std::string result(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);
        
        return result;
    }
    
    static std::string generate_salt() {
        unsigned char salt[16];
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        for (int i = 0; i < 16; i++) {
            salt[i] = dis(gen);
        }
        
        return base64_encode(salt, 16);
    }
    
    static std::string hash_password(const std::string& password, const std::string& salt) {
        std::string combined = salt + password;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    static std::string sanitize_input(const std::string& input) {
        std::string sanitized;
        for (char c : input) {
            if (isalnum(c) || c == '_') {
                sanitized += c;
            }
            if (sanitized.length() >= 50) break;
        }
        return sanitized;
    }
    
    class ChatServer {
    private:
        int port;
        std::string user_file;
        
        void initialize_user_file() {
            std::ofstream file(user_file);
            std::vector<std::string> users = {"alice", "bob", "charlie", "david", "eve"};
            std::vector<std::string> passwords = {"password123", "secure456", "test789", "demo000", "chat111"};
            
            for (size_t i = 0; i < users.size(); i++) {
                std::string salt = generate_salt();
                std::string hashed = hash_password(passwords[i], salt);
                file << users[i] << ":" << salt << ":" << hashed << "\\n";
            }
            file.close();
        }
        
        bool authenticate_user(const std::string& username, const std::string& password) {
            std::ifstream file(user_file);
            if (!file.good()) {
                initialize_user_file();
                file.open(user_file);
            }
            
            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string stored_user, stored_salt, stored_hash;
                
                if (std::getline(iss, stored_user, ':') &&
                    std::getline(iss, stored_salt, ':') &&
                    std::getline(iss, stored_hash)) {
                    
                    if (stored_user == username) {
                        std::string hashed = hash_password(password, stored_salt);
                        file.close();
                        return hashed == stored_hash;
                    }
                }
            }
            file.close();
            return false;
        }
        
        void handle_client(socket_t client_socket) {
            char buffer[1024] = {0};
            int bytes_read = recv(client_socket, buffer, 1024, 0);
            
            if (bytes_read > 0) {
                std::string request(buffer);
                std::string response;
                
                if (request.substr(0, 6) == "LOGIN:") {
                    std::string creds = request.substr(6);
                    size_t colon_pos = creds.find(':');
                    
                    if (colon_pos != std::string::npos) {
                        std::string username = sanitize_input(creds.substr(0, colon_pos));
                        std::string password = creds.substr(colon_pos + 1);
                        
                        if (authenticate_user(username, password)) {
                            response = "SUCCESS:Login successful";
                        } else {
                            response = "FAILURE:Invalid credentials";
                        }
                    } else {
                        response = "FAILURE:Invalid request format";
                    }
                } else {
                    response = "FAILURE:Unknown request";
                }
                
                send(client_socket, response.c_str(), response.length(), 0);
            }
            
            closesocket(client_socket);
        }
        
    public:
        ChatServer(int p = 8888) : port(p), user_file("users.txt") {
            #ifdef _WIN32
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            #endif
        }
        
        void start() {
            socket_t server_socket = socket(AF_INET, SOCK_STREAM, 0);
            
            sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_addr.s_addr = INADDR_ANY;
            server_addr.sin_port = htons(port);
            
            int opt = 1;
            setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
            
            bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
            listen(server_socket, 5);
            
            std::cout << "Server started on port " << port << std::endl;
            
            while (true) {
                sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                socket_t client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_len);
                
                std::thread(&ChatServer::handle_client, this, client_socket).detach();
            }
        }
        
        ~ChatServer() {
            #ifdef _WIN32
            WSACleanup();
            #endif
        }
    };
    
    class ChatClient {
    private:
        std::string server_address;
        int server_port;
        
    public:
        ChatClient(const std::string& addr = "127.0.0.1", int port = 8888) 
            : server_address(addr), server_port(port) {
            #ifdef _WIN32
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
            #endif
        }
        
        std::string login(const std::string& username, const std::string& password) {
            socket_t client_socket = socket(AF_INET, SOCK_STREAM, 0);
            
            sockaddr_in server_addr;
            server_addr.sin_family = AF_INET;
            server_addr.sin_port = htons(server_port);
            inet_pton(AF_INET, server_address.c_str(), &server_addr.sin_addr);
            
            if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
                return "FAILURE:Connection error";
            }
            
            std::string request = "LOGIN:" + username + ":" + password;
            send(client_socket, request.c_str(), request.length(), 0);
            
            char buffer[1024] = {0};
            recv(client_socket, buffer, 1024, 0);
            
            closesocket(client_socket);
            return std::string(buffer);
        }
        
        ~ChatClient() {
            #ifdef _WIN32
            WSACleanup();
            #endif
        }
    };
};

int main() {
    // Start server in separate thread
    std::thread server_thread([]() {
        Task7::ChatServer server;
        server.start();
    });
    server_thread.detach();
    
    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Test cases
    Task7::ChatClient client;
    
    std::cout << "Test Case 1 - Valid login (alice):" << std::endl;
    std::cout << client.login("alice", "password123") << std::endl;
    
    std::cout << "\\nTest Case 2 - Valid login (bob):" << std::endl;
    std::cout << client.login("bob", "secure456") << std::endl;
    
    std::cout << "\\nTest Case 3 - Invalid password:" << std::endl;
    std::cout << client.login("alice", "wrongpassword") << std::endl;
    
    std::cout << "\\nTest Case 4 - Non-existent user:" << std::endl;
    std::cout << client.login("hacker", "password") << std::endl;
    
    std::cout << "\\nTest Case 5 - Valid login (charlie):" << std::endl;
    std::cout << client.login("charlie", "test789") << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    return 0;
}
