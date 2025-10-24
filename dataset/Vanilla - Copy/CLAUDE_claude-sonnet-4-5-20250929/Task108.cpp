
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <sstream>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
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

std::map<std::string, std::string> objectStore;
std::mutex storeMutex;

void storeObject(const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(storeMutex);
    objectStore[key] = value;
}

std::string retrieveObject(const std::string& key) {
    std::lock_guard<std::mutex> lock(storeMutex);
    auto it = objectStore.find(key);
    if (it != objectStore.end()) {
        return it->second;
    }
    return "";
}

bool deleteObject(const std::string& key) {
    std::lock_guard<std::mutex> lock(storeMutex);
    return objectStore.erase(key) > 0;
}

std::set<std::string> listKeys() {
    std::lock_guard<std::mutex> lock(storeMutex);
    std::set<std::string> keys;
    for (const auto& pair : objectStore) {
        keys.insert(pair.first);
    }
    return keys;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void handleClient(SOCKET clientSocket) {
    char buffer[4096];
    int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\\0';
        std::string request(buffer);
        std::vector<std::string> parts = split(request, '|');
        std::string response;
        
        if (parts.size() > 0) {
            std::string command = parts[0];
            
            if (command == "STORE" && parts.size() >= 3) {
                storeObject(parts[1], parts[2]);
                response = "OK|Object stored";
            } else if (command == "RETRIEVE" && parts.size() >= 2) {
                std::string value = retrieveObject(parts[1]);
                response = value.empty() ? "ERROR|Key not found" : "OK|" + value;
            } else if (command == "DELETE" && parts.size() >= 2) {
                bool deleted = deleteObject(parts[1]);
                response = deleted ? "OK|Object deleted" : "ERROR|Key not found";
            } else if (command == "LIST") {
                std::set<std::string> keys = listKeys();
                std::stringstream ss;
                ss << "OK|";
                for (const auto& key : keys) {
                    ss << key << " ";
                }
                response = ss.str();
            } else {
                response = "ERROR|Unknown command or invalid parameters";
            }
        }
        
        send(clientSocket, response.c_str(), response.length(), 0);
    }
    
    closesocket(clientSocket);
}

void startServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);
    
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed" << std::endl;
        closesocket(serverSocket);
        return;
    }
    
    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed" << std::endl;
        closesocket(serverSocket);
        return;
    }
    
    std::cout << "Server listening on port " << port << std::endl;
    
    while (true) {
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrLen);
        
        if (clientSocket != INVALID_SOCKET) {
            std::thread(handleClient, clientSocket).detach();
        }
    }
    
    closesocket(serverSocket);
#ifdef _WIN32
    WSACleanup();
#endif
}

int main() {
    // Test cases
    std::cout << "=== Test Case 1: Store and Retrieve ===" << std::endl;
    storeObject("key1", "Hello World");
    std::cout << "Retrieved: " << retrieveObject("key1") << std::endl;
    
    std::cout << "\\n=== Test Case 2: Store Multiple Objects ===" << std::endl;
    storeObject("key2", "12345");
    storeObject("key3", "[1.1, 2.2, 3.3]");
    std::cout << "Retrieved key2: " << retrieveObject("key2") << std::endl;
    std::cout << "Retrieved key3: " << retrieveObject("key3") << std::endl;
    
    std::cout << "\\n=== Test Case 3: Update Object ===" << std::endl;
    storeObject("key1", "Updated Value");
    std::cout << "Updated key1: " << retrieveObject("key1") << std::endl;
    
    std::cout << "\\n=== Test Case 4: Delete Object ===" << std::endl;
    bool deleted = deleteObject("key2");
    std::cout << "Deleted key2: " << (deleted ? "true" : "false") << std::endl;
    std::cout << "Retrieve deleted key2: " << retrieveObject("key2") << std::endl;
    
    std::cout << "\\n=== Test Case 5: List All Keys ===" << std::endl;
    std::set<std::string> keys = listKeys();
    std::cout << "All keys: ";
    for (const auto& key : keys) {
        std::cout << key << " ";
    }
    std::cout << std::endl;
    
    // Start server in a separate thread
    std::thread serverThread(startServer, 8080);
    serverThread.detach();
    
    std::cout << "\\nServer started on port 8080. Press Enter to stop." << std::endl;
    std::cin.get();
    
    return 0;
}
