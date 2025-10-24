
#include <iostream>
#include <string>
#include <cstring>
#include <thread>
#include <chrono>

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

using namespace std;

class Task96 {
private:
    static const int PORT = 8888;
    
public:
    static void startServer(int port) {
        #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            cerr << "WSAStartup failed" << endl;
            return;
        }
        #endif
        
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            cerr << "Could not create socket" << endl;
            #ifdef _WIN32
            WSACleanup();
            #endif
            return;
        }
        
        int opt = 1;
        setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt));
        
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_port = htons(port);
        
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            cerr << "Bind failed" << endl;
            closesocket(serverSocket);
            #ifdef _WIN32
            WSACleanup();
            #endif
            return;
        }
        
        if (listen(serverSocket, 5) == SOCKET_ERROR) {
            cerr << "Listen failed" << endl;
            closesocket(serverSocket);
            #ifdef _WIN32
            WSACleanup();
            #endif
            return;
        }
        
        cout << "Server started on port " << port << endl;
        
        while (true) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
            
            if (clientSocket == INVALID_SOCKET) {
                continue;
            }
            
            cout << "Client connected" << endl;
            
            char buffer[1024];
            while (true) {
                memset(buffer, 0, sizeof(buffer));
                int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
                
                if (bytesRead <= 0) {
                    break;
                }
                
                cout << "Received: " << buffer;
                send(clientSocket, buffer, bytesRead, 0);
                
                if (strstr(buffer, "QUIT") != NULL) {
                    break;
                }
            }
            
            closesocket(clientSocket);
            cout << "Client disconnected" << endl;
        }
        
        closesocket(serverSocket);
        #ifdef _WIN32
        WSACleanup();
        #endif
    }
    
    static string sendMessage(int port, const string& message) {
        #ifdef _WIN32
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        #endif
        
        SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == INVALID_SOCKET) {
            #ifdef _WIN32
            WSACleanup();
            #endif
            return "Error: Could not create socket";
        }
        
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_port = htons(port);
        
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            closesocket(clientSocket);
            #ifdef _WIN32
            WSACleanup();
            #endif
            return "Error: Connection failed";
        }
        
        string msg = message + "\\n";
        send(clientSocket, msg.c_str(), msg.length(), 0);
        
        char buffer[1024];
        memset(buffer, 0, sizeof(buffer));
        recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        closesocket(clientSocket);
        #ifdef _WIN32
        WSACleanup();
        #endif
        
        string response(buffer);
        if (!response.empty() && response.back() == '\\n') {
            response.pop_back();
        }
        return response;
    }
};

int main() {
    // Start server in a separate thread
    thread serverThread([]() { Task96::startServer(8888); });
    serverThread.detach();
    
    // Wait for server to start
    this_thread::sleep_for(chrono::seconds(1));
    
    // Test cases
    cout << "\\n=== Running Test Cases ===\\n" << endl;
    
    // Test 1: Simple message
    cout << "Test 1: Simple message" << endl;
    string response1 = Task96::sendMessage(8888, "Hello, Server!");
    cout << "Sent: Hello, Server!" << endl;
    cout << "Echo: " << response1 << endl << endl;
    
    // Test 2: Numeric message
    cout << "Test 2: Numeric message" << endl;
    string response2 = Task96::sendMessage(8888, "12345");
    cout << "Sent: 12345" << endl;
    cout << "Echo: " << response2 << endl << endl;
    
    // Test 3: Special characters
    cout << "Test 3: Special characters" << endl;
    string response3 = Task96::sendMessage(8888, "Test@#$%^&*()");
    cout << "Sent: Test@#$%^&*()" << endl;
    cout << "Echo: " << response3 << endl << endl;
    
    // Test 4: Space message
    cout << "Test 4: Space message" << endl;
    string response4 = Task96::sendMessage(8888, " ");
    cout << "Sent: ' '" << endl;
    cout << "Echo: '" << response4 << "'" << endl << endl;
    
    // Test 5: Long message
    cout << "Test 5: Long message" << endl;
    string longMsg = "This is a longer message to test the echo functionality!";
    string response5 = Task96::sendMessage(8888, longMsg);
    cout << "Sent: " << longMsg << endl;
    cout << "Echo: " << response5 << endl << endl;
    
    cout << "=== All Tests Completed ===" << endl;
    
    this_thread::sleep_for(chrono::seconds(1));
    
    return 0;
}
