
#include <iostream>
#include <string>
#include <map>
#include <set>
#include <thread>
#include <mutex>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
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

using namespace std;

class Task108 {
private:
    static const int PORT = 8080;
    static const string SECRET_KEY;
    static map<string, string> serverObjects;
    static set<string> authenticatedClients;
    static mutex objectsMutex;
    static mutex clientsMutex;
    static bool serverRunning;
    
    static void initSocket() {
        #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        #endif
    }
    
    static void cleanupSocket() {
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
    
    static string sanitizeInput(const string& input) {
        string result;
        for (char c : input) {
            if (isalnum(c) || c == '_' || c == '-') {
                result += c;
            }
        }
        return result;
    }
    
    static vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        stringstream ss(str);
        string token;
        while (getline(ss, token, delimiter)) {
            tokens.push_back(token);
        }
        return tokens;
    }
    
    static bool authenticate(const string& token) {
        return token == SECRET_KEY;
    }
    
    static string processCommand(const vector<string>& parts) {
        if (parts.empty()) return "ERROR|Empty command";
        
        string command = parts[0];
        
        try {
            if (command == "GET" && parts.size() == 2) {
                string key = sanitizeInput(parts[1]);
                lock_guard<mutex> lock(objectsMutex);
                auto it = serverObjects.find(key);
                if (it != serverObjects.end()) {
                    return "SUCCESS|" + it->second;
                }
                return "ERROR|Object not found";
            }
            else if (command == "SET" && parts.size() == 3) {
                string key = sanitizeInput(parts[1]);
                string value = sanitizeInput(parts[2]);
                lock_guard<mutex> lock(objectsMutex);
                serverObjects[key] = value;
                return "SUCCESS|Object set";
            }
            else if (command == "DELETE" && parts.size() == 2) {
                string key = sanitizeInput(parts[1]);
                lock_guard<mutex> lock(objectsMutex);
                serverObjects.erase(key);
                return "SUCCESS|Object deleted";
            }
            else if (command == "LIST") {
                lock_guard<mutex> lock(objectsMutex);
                string keys;
                for (auto& pair : serverObjects) {
                    if (!keys.empty()) keys += ",";
                    keys += pair.first;
                }
                return "SUCCESS|" + keys;
            }
            else {
                return "ERROR|Unknown command";
            }
        } catch (...) {
            return "ERROR|Command processing failed";
        }
    }
    
    static void handleClient(SOCKET clientSocket, string clientId) {
        char buffer[1024] = {0};
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesRead <= 0) {
            closesocket(clientSocket);
            return;
        }
        
        string request(buffer);
        vector<string> parts = split(request, '|');
        
        if (parts.empty()) {
            closesocket(clientSocket);
            return;
        }
        
        string response;
        string command = parts[0];
        
        if (command == "AUTH" && parts.size() == 2) {
            if (authenticate(parts[1])) {
                lock_guard<mutex> lock(clientsMutex);
                authenticatedClients.insert(clientId);
                response = "AUTH_SUCCESS";
            } else {
                response = "AUTH_FAILED";
            }
        } else {
            lock_guard<mutex> lock(clientsMutex);
            if (authenticatedClients.find(clientId) == authenticatedClients.end()) {
                response = "ERROR|Not authenticated";
            } else {
                response = processCommand(parts);
            }
        }
        
        send(clientSocket, response.c_str(), response.length(), 0);
        closesocket(clientSocket);
    }
    
public:
    static void startServer() {
        initSocket();
        
        serverObjects["object1"] = "value1";
        serverObjects["object2"] = "value2";
        serverRunning = true;
        
        thread serverThread([]() {
            SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
            if (serverSocket == INVALID_SOCKET) {
                cerr << "Socket creation failed" << endl;
                return;
            }
            
            int opt = 1;
            setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
            
            sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_addr.s_addr = INADDR_ANY;
            serverAddr.sin_port = htons(PORT);
            
            if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
                cerr << "Bind failed" << endl;
                closesocket(serverSocket);
                return;
            }
            
            if (listen(serverSocket, 5) == SOCKET_ERROR) {
                cerr << "Listen failed" << endl;
                closesocket(serverSocket);
                return;
            }
            
            cout << "Server started on port " << PORT << endl;
            
            while (serverRunning) {
                sockaddr_in clientAddr;
                socklen_t clientLen = sizeof(clientAddr);
                SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);
                
                if (clientSocket != INVALID_SOCKET) {
                    string clientId = inet_ntoa(clientAddr.sin_addr);
                    thread(handleClient, clientSocket, clientId).detach();
                }
            }
            
            closesocket(serverSocket);
            cleanupSocket();
        });
        
        serverThread.detach();
    }
    
    static string sendRequest(const string& host, int port, const string& request) {
        initSocket();
        
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            return "ERROR|Socket creation failed";
        }
        
        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = inet_addr(host.c_str());
        
        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(clientSocket);
            return "ERROR|Connection failed";
        }
        
        send(clientSocket, request.c_str(), request.length(), 0);
        
        char buffer[1024] = {0};
        int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        closesocket(clientSocket);
        
        if (bytesRead > 0) {
            return string(buffer);
        }
        
        return "ERROR|No response";
    }
    
    static void stopServer() {
        serverRunning = false;
    }
};

const string Task108::SECRET_KEY = "MySecretKey12345";
map<string, string> Task108::serverObjects;
set<string> Task108::authenticatedClients;
mutex Task108::objectsMutex;
mutex Task108::clientsMutex;
bool Task108::serverRunning = false;

int main() {
    cout << "=== Secure Remote Object Access System ===" << endl << endl;
    
    // Start server
    Task108::startServer();
    
    // Wait for server to start
    this_thread::sleep_for(chrono::seconds(1));
    
    // Test Case 1: Authentication
    cout << "Test 1 - Authentication:" << endl;
    string response = Task108::sendRequest("127.0.0.1", 8080, 
                                          "AUTH|MySecretKey12345");
    cout << "Response: " << response << endl << endl;
    
    // Test Case 2: Get object
    cout << "Test 2 - Get object:" << endl;
    response = Task108::sendRequest("127.0.0.1", 8080, "GET|object1");
    cout << "Response: " << response << endl << endl;
    
    // Test Case 3: Set object
    cout << "Test 3 - Set object:" << endl;
    response = Task108::sendRequest("127.0.0.1", 8080, "SET|object3|value3");
    cout << "Response: " << response << endl << endl;
    
    // Test Case 4: List objects
    cout << "Test 4 - List objects:" << endl;
    response = Task108::sendRequest("127.0.0.1", 8080, "LIST");
    cout << "Response: " << response << endl << endl;
    
    // Test Case 5: Delete object
    cout << "Test 5 - Delete object:" << endl;
    response = Task108::sendRequest("127.0.0.1", 8080, "DELETE|object2");
    cout << "Response: " << response << endl << endl;
    
    Task108::stopServer();
    this_thread::sleep_for(chrono::seconds(1));
    
    return 0;
}
