
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <cstring>
#include <openssl/sha.h>
#include <openssl/rand.h>

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

class Task7 {
private:
    static const int PORT = 8080;
    static const int MAX_LOGIN_ATTEMPTS = 3;
    static const long LOCKOUT_TIME = 300; // 5 minutes in seconds
    
    static std::map<std::string, int> loginAttempts;
    static std::map<std::string, long long> lockedAccounts;
    static std::mutex mtx;
    
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string ret;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        
        while (length--) {
            char_array_3[i++] = *(buffer++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for(i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];
            
            while((i++ < 3))
                ret += '=';
        }
        
        return ret;
    }
    
    static std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, sizeof(salt));
        return base64_encode(salt, sizeof(salt));
    }
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, salt.c_str(), salt.length());
        SHA256_Update(&sha256, password.c_str(), password.length());
        SHA256_Final(hash, &sha256);
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    static void initializeUsersFile() {
        std::ofstream file("users.dat");
        if (!file.is_open()) return;
        
        std::vector<std::pair<std::string, std::string>> users = {
            {"user1", "password1"},
            {"user2", "password2"},
            {"admin", "admin123"},
            {"testuser", "test123"},
            {"alice", "alice456"}
        };
        
        for (const auto& user : users) {
            std::string salt = generateSalt();
            std::string hash = hashPassword(user.second, salt);
            file << user.first << ":" << salt << ":" << hash << "\\n";
        }
        file.close();
    }
    
    static bool isValidUsername(const std::string& username) {
        if (username.empty() || username.length() > 50) return false;
        return std::all_of(username.begin(), username.end(), 
            [](char c) { return std::isalnum(c) || c == '_'; });
    }
    
    static std::string sanitizeInput(const std::string& input) {
        std::string result;
        for (char c : input) {
            if (std::isalnum(c) || c == '_') {
                result += c;
            }
        }
        return result;
    }
    
    static long long getCurrentTime() {
        return std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
    }
    
    static bool isAccountLocked(const std::string& username) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = lockedAccounts.find(username);
        if (it != lockedAccounts.end()) {
            if (getCurrentTime() < it->second) {
                return true;
            } else {
                lockedAccounts.erase(it);
                loginAttempts.erase(username);
            }
        }
        return false;
    }
    
    static void incrementLoginAttempts(const std::string& username) {
        std::lock_guard<std::mutex> lock(mtx);
        int attempts = ++loginAttempts[username];
        if (attempts >= MAX_LOGIN_ATTEMPTS) {
            lockedAccounts[username] = getCurrentTime() + LOCKOUT_TIME;
        }
    }
    
    static bool authenticateUser(const std::string& username, const std::string& password) {
        std::ifstream file("users.dat");
        if (!file.is_open()) {
            initializeUsersFile();
            file.open("users.dat");
        }
        
        std::string line;
        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string storedUsername, storedSalt, storedHash;
            
            if (std::getline(iss, storedUsername, ':') &&
                std::getline(iss, storedSalt, ':') &&
                std::getline(iss, storedHash)) {
                
                if (storedUsername == username) {
                    std::string computedHash = hashPassword(password, storedSalt);
                    return computedHash == storedHash;
                }
            }
        }
        return false;
    }
    
    static std::vector<std::string> split(const std::string& str, char delimiter, int maxParts = -1) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        
        while (std::getline(tokenStream, token, delimiter)) {
            tokens.push_back(token);
            if (maxParts > 0 && tokens.size() >= static_cast<size_t>(maxParts - 1)) {
                std::string remaining;
                std::getline(tokenStream, remaining);
                if (!remaining.empty()) {
                    tokens.push_back(remaining);
                }
                break;
            }
        }
        return tokens;
    }
    
    static std::string processRequest(const std::string& request) {
        if (request.empty() || request.length() > 1000) {
            return "ERROR:Invalid request format";
        }
        
        std::vector<std::string> parts = split(request, ':', 3);
        if (parts.size() != 3 || parts[0] != "LOGIN") {
            return "ERROR:Invalid request format";
        }
        
        std::string username = sanitizeInput(parts[1]);
        std::string password = parts[2];
        
        if (!isValidUsername(username)) {
            return "ERROR:Invalid username format";
        }
        
        if (isAccountLocked(username)) {
            return "ERROR:Account temporarily locked";
        }
        
        if (authenticateUser(username, password)) {
            std::lock_guard<std::mutex> lock(mtx);
            loginAttempts.erase(username);
            return "SUCCESS:Login successful";
        } else {
            incrementLoginAttempts(username);
            return "ERROR:Invalid credentials";
        }
    }
    
    static void handleClient(SOCKET clientSocket) {
        char buffer[1024] = {0};
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        std::string response;
        if (bytesRead > 0) {
            buffer[bytesRead] = '\\0';
            std::string request(buffer);
            // Remove newline characters
            request.erase(std::remove(request.begin(), request.end(), '\\n'), request.end());
            request.erase(std::remove(request.begin(), request.end(), '\\r'), request.end());
            response = processRequest(request);
        } else {
            response = "ERROR:Invalid request";
        }
        
        response += "\\n";
        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }
    
public:
    static void startServer() {
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed\\n";
            return;
        }
#endif
        
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed\\n";
            return;
        }
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
        
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);
        
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::cerr << "Bind failed\\n";
            closesocket(serverSocket);
            return;
        }
        
        if (listen(serverSocket, 5) == SOCKET_ERROR) {
            std::cerr << "Listen failed\\n";
            closesocket(serverSocket);
            return;
        }
        
        std::cout << "Server started on port " << PORT << std::endl;
        
        while (true) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            
            if (clientSocket != INVALID_SOCKET) {
                std::thread clientThread(handleClient, clientSocket);
                clientThread.detach();
            }
        }
        
        closesocket(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
    }
    
    static std::string login(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty() || 
            username.find('\\n') != std::string::npos || password.find('\\n') != std::string::npos) {
            return "ERROR:Invalid input format";
        }
        
        if (username.length() > 50 || password.length() > 100) {
            return "ERROR:Input too long";
        }
        
        std::string sanitizedUsername = sanitizeInput(username);
        std::string request = "LOGIN:" + sanitizedUsername + ":" + password;
        
#ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
        
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            return "ERROR:Socket creation failed";
        }
        
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(PORT);
        
#ifdef _WIN32
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
#else
        inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);
#endif
        
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(clientSocket);
            return "ERROR:Connection failed";
        }
        
        request += "\\n";
        send(clientSocket, request.c_str(), request.length(), 0);
        
        char buffer[1024] = {0};
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        closesocket(clientSocket);
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\\0';
            std::string response(buffer);
            response.erase(std::remove(response.begin(), response.end(), '\\n'), response.end());
            response.erase(std::remove(response.begin(), response.end(), '\\r'), response.end());
            return response;
        }
        
        return "ERROR:No response from server";
    }
};

std::map<std::string, int> Task7::loginAttempts;
std::map<std::string, long long> Task7::lockedAccounts;
std::mutex Task7::mtx;

int main() {
    // Start server in a separate thread
    std::thread serverThread(Task7::startServer);
    serverThread.detach();
    
    // Wait for server to start
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Test cases
    std::cout << "=== Running Test Cases ===\\n\\n";
    
    // Test case 1: Valid login
    std::cout << "Test 1 - Valid user login:\\n";
    std::string response1 = Task7::login("user1", "password1");
    std::cout << "Response: " << response1 << "\\n\\n";
    
    // Test case 2: Invalid password
    std::cout << "Test 2 - Invalid password:\\n";
    std::string response2 = Task7::login("user1", "wrongpassword");
    std::cout << "Response: " << response2 << "\\n\\n";
    
    // Test case 3: Non-existent user
    std::cout << "Test 3 - Non-existent user:\\n";
    std::string response3 = Task7::login("nonexistent", "password");
    std::cout << "Response: " << response3 << "\\n\\n";
    
    // Test case 4: Valid admin login
    std::cout << "Test 4 - Admin login:\\n";
    std::string response4 = Task7::login("admin", "admin123");
    std::cout << "Response: " << response4 << "\\n\\n";
    
    // Test case 5: Empty credentials
    std::cout << "Test 5 - Empty credentials:\\n";
    std::string response5 = Task7::login("", "");
    std::cout << "Response: " << response5 << "\\n\\n";
    
    // Keep the program running
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}
