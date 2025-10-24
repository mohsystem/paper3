
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <cstring>
#include <ctime>
#include <sstream>
#include <algorithm>
#include <random>
#include <iomanip>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

class Task108 {
private:
    static const int PORT = 8443;
    static const int MAX_CONNECTIONS = 10;
    static const int SESSION_TIMEOUT = 300;
    
    class SecureObjectStore {
    private:
        std::map<std::string, std::string> store;
        std::map<std::string, time_t> sessionTokens;
        std::mutex mtx;
        
        std::string hashPassword(const std::string& password) {
            // Simple hash for demonstration (use proper crypto library in production)
            std::hash<std::string> hasher;
            std::stringstream ss;
            ss << std::hex << hasher(password);
            return ss.str();
        }
        
        std::string generateSecureToken() {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, 255);
            
            std::stringstream ss;
            for (int i = 0; i < 32; i++) {
                ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
            }
            return ss.str();
        }
        
        bool isValidKey(const std::string& key) {
            if (key.empty() || key.length() > 50) return false;
            for (char c : key) {
                if (!std::isalnum(c) && c != '_' && c != '-') return false;
            }
            return true;
        }
        
        bool isValidValue(const std::string& value) {
            return !value.empty() && value.length() <= 1000;
        }
        
        std::string sanitize(const std::string& input) {
            std::string result;
            for (char c : input) {
                if (std::isalnum(c) || c == '_' || c == '-' || c == ' ') {
                    result += c;
                }
            }
            return result;
        }
        
    public:
        bool authenticate(const std::string& username, const std::string& password) {
            return username == "admin" && hashPassword(password) == hashPassword("admin123");
        }
        
        std::string createSession(const std::string& username) {
            std::lock_guard<std::mutex> lock(mtx);
            std::string token = generateSecureToken();
            sessionTokens[token] = time(nullptr);
            return token;
        }
        
        bool validateSession(const std::string& token) {
            std::lock_guard<std::mutex> lock(mtx);
            auto it = sessionTokens.find(token);
            if (it == sessionTokens.end()) return false;
            if (time(nullptr) - it->second > SESSION_TIMEOUT) {
                sessionTokens.erase(it);
                return false;
            }
            return true;
        }
        
        std::string put(const std::string& key, const std::string& value, const std::string& token) {
            if (!validateSession(token)) return "ERROR: Invalid session";
            if (!isValidKey(key)) return "ERROR: Invalid key format";
            if (!isValidValue(value)) return "ERROR: Invalid value";
            
            std::lock_guard<std::mutex> lock(mtx);
            store[sanitize(key)] = sanitize(value);
            return "SUCCESS: Object stored";
        }
        
        std::string get(const std::string& key, const std::string& token) {
            if (!validateSession(token)) return "ERROR: Invalid session";
            if (!isValidKey(key)) return "ERROR: Invalid key format";
            
            std::lock_guard<std::mutex> lock(mtx);
            auto it = store.find(sanitize(key));
            return it != store.end() ? it->second : "ERROR: Key not found";
        }
        
        std::string deleteKey(const std::string& key, const std::string& token) {
            if (!validateSession(token)) return "ERROR: Invalid session";
            if (!isValidKey(key)) return "ERROR: Invalid key format";
            
            std::lock_guard<std::mutex> lock(mtx);
            auto it = store.find(sanitize(key));
            if (it != store.end()) {
                store.erase(it);
                return "SUCCESS: Object deleted";
            }
            return "ERROR: Key not found";
        }
        
        std::string list(const std::string& token) {
            if (!validateSession(token)) return "ERROR: Invalid session";
            
            std::lock_guard<std::mutex> lock(mtx);
            std::string result = "KEYS: ";
            for (auto it = store.begin(); it != store.end(); ++it) {
                if (it != store.begin()) result += ",";
                result += it->first;
            }
            return result;
        }
    };
    
    static std::vector<std::string> split(const std::string& str, char delimiter) {
        std::vector<std::string> tokens;
        std::stringstream ss(str);
        std::string token;
        while (std::getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
    
    static void handleClient(SOCKET clientSocket, SecureObjectStore* store) {
        std::string sessionToken;
        char buffer[4096];
        
        const char* welcome = "Welcome to Secure Object Store. Commands: AUTH|PUT|GET|DELETE|LIST|QUIT\\n";
        send(clientSocket, welcome, strlen(welcome), 0);
        
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
            
            if (bytesReceived <= 0) break;
            
            std::string command(buffer);
            command.erase(command.find_last_not_of("\\r\\n") + 1);
            
            if (command.length() > 2000) {
                const char* error = "ERROR: Command too long\\n";
                send(clientSocket, error, strlen(error), 0);
                continue;
            }
            
            std::vector<std::string> parts = split(command, '|');
            if (parts.empty()) {
                const char* error = "ERROR: Invalid command\\n";
                send(clientSocket, error, strlen(error), 0);
                continue;
            }
            
            std::string cmd = parts[0];
            std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
            std::string response;
            
            if (cmd == "AUTH") {
                if (parts.size() != 3) {
                    response = "ERROR: Usage: AUTH|username|password\\n";
                } else if (store->authenticate(parts[1], parts[2])) {
                    sessionToken = store->createSession(parts[1]);
                    response = "SUCCESS: Authenticated. Token: " + sessionToken + "\\n";
                } else {
                    response = "ERROR: Authentication failed\\n";
                }
            } else if (cmd == "PUT") {
                if (parts.size() != 3) {
                    response = "ERROR: Usage: PUT|key|value\\n";
                } else if (sessionToken.empty()) {
                    response = "ERROR: Not authenticated\\n";
                } else {
                    response = store->put(parts[1], parts[2], sessionToken) + "\\n";
                }
            } else if (cmd == "GET") {
                if (parts.size() != 2) {
                    response = "ERROR: Usage: GET|key\\n";
                } else if (sessionToken.empty()) {
                    response = "ERROR: Not authenticated\\n";
                } else {
                    response = store->get(parts[1], sessionToken) + "\\n";
                }
            } else if (cmd == "DELETE") {
                if (parts.size() != 2) {
                    response = "ERROR: Usage: DELETE|key\\n";
                } else if (sessionToken.empty()) {
                    response = "ERROR: Not authenticated\\n";
                } else {
                    response = store->deleteKey(parts[1], sessionToken) + "\\n";
                }
            } else if (cmd == "LIST") {
                if (sessionToken.empty()) {
                    response = "ERROR: Not authenticated\\n";
                } else {
                    response = store->list(sessionToken) + "\\n";
                }
            } else if (cmd == "QUIT") {
                response = "Goodbye\\n";
                send(clientSocket, response.c_str(), response.length(), 0);
                break;
            } else {
                response = "ERROR: Unknown command\\n";
            }
            
            send(clientSocket, response.c_str(), response.length(), 0);
        }
        
        closesocket(clientSocket);
    }
    
public:
    static void startServer() {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        
        SecureObjectStore store;
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
            return;
        }
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);
        
        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed" << std::endl;
            closesocket(serverSocket);
            return;
        }
        
        if (listen(serverSocket, MAX_CONNECTIONS) == SOCKET_ERROR) {
            std::cerr << "Listen failed" << std::endl;
            closesocket(serverSocket);
            return;
        }
        
        std::cout << "Secure Object Store Server started on port " << PORT << std::endl;
        
        while (true) {
            sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket != INVALID_SOCKET) {
                std::thread clientThread(handleClient, clientSocket, &store);
                clientThread.detach();
            }
        }
        
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    static std::string sendCommand(const std::string& host, int port, const std::string& command) {
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) return "ERROR: Socket creation failed";
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
        
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(clientSocket);
            return "ERROR: Connection failed";
        }
        
        char buffer[4096];
        recv(clientSocket, buffer, sizeof(buffer), 0);
        
        std::string cmd = command + "\\n";
        send(clientSocket, cmd.c_str(), cmd.length(), 0);
        
        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, sizeof(buffer), 0);
        
        closesocket(clientSocket);
#ifdef _WIN32
        WSACleanup();
#endif
        
        std::string response(buffer);
        response.erase(response.find_last_not_of("\\r\\n") + 1);
        return response;
    }
};

int main() {
    std::thread serverThread(Task108::startServer);
    serverThread.detach();
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "=== Test Case 1: Authentication ===" << std::endl;
    std::cout << Task108::sendCommand("127.0.0.1", 8443, "AUTH|admin|admin123") << std::endl;
    
    std::cout << "\\n=== Test Case 2: Put Object ===" << std::endl;
    std::cout << Task108::sendCommand("127.0.0.1", 8443, "AUTH|admin|admin123") << std::endl;
    
    std::cout << "\\n=== Test Case 3: Get Object ===" << std::endl;
    std::cout << Task108::sendCommand("127.0.0.1", 8443, "AUTH|admin|admin123") << std::endl;
    
    std::cout << "\\n=== Test Case 4: List Objects ===" << std::endl;
    std::cout << Task108::sendCommand("127.0.0.1", 8443, "AUTH|admin|admin123") << std::endl;
    
    std::cout << "\\n=== Test Case 5: Delete Object ===" << std::endl;
    std::cout << Task108::sendCommand("127.0.0.1", 8443, "AUTH|admin|admin123") << std::endl;
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}
