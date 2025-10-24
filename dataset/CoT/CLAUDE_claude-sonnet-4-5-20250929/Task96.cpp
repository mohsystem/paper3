
#include <iostream>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
    #define SOCKET_ERROR_CHECK(x) ((x) == SOCKET_ERROR)
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define SOCKET int
    #define INVALID_SOCKET -1
    #define CLOSE_SOCKET close
    #define SOCKET_ERROR_CHECK(x) ((x) < 0)
#endif

const int BUFFER_SIZE = 8192;
const int TIMEOUT_SEC = 30;
const int MIN_PORT = 1024;
const int MAX_PORT = 65535;

class Task96 {
public:
    static void startServer(int port) {
        if (port < MIN_PORT || port > MAX_PORT) {
            std::cerr << "Port must be between " << MIN_PORT << " and " << MAX_PORT << std::endl;
            return;
        }

#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed" << std::endl;
            return;
        }
#endif

        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::cerr << "Socket creation failed" << std::endl;
#ifdef _WIN32
            WSACleanup();
#endif
            return;
        }

        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
            std::cerr << "Setsockopt failed" << std::endl;
        }

        sockaddr_in serverAddr;
        std::memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Bind failed" << std::endl;
            CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
            WSACleanup();
#endif
            return;
        }

        if (listen(serverSocket, 5) < 0) {
            std::cerr << "Listen failed" << std::endl;
            CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
            WSACleanup();
#endif
            return;
        }

        std::cout << "Server listening on port " << port << std::endl;

        while (true) {
            sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);

            if (clientSocket == INVALID_SOCKET) {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }

            std::cout << "Client connected" << std::endl;
            handleClient(clientSocket);
            CLOSE_SOCKET(clientSocket);
        }

        CLOSE_SOCKET(serverSocket);
#ifdef _WIN32
        WSACleanup();
#endif
    }

private:
    static void handleClient(SOCKET clientSocket) {
        char buffer[BUFFER_SIZE];
        
        while (true) {
            std::memset(buffer, 0, BUFFER_SIZE);
            int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            
            if (bytesRead <= 0) {
                break;
            }

            buffer[bytesRead] = '\\0';
            std::cout << "Received: " << buffer;
            
            if (send(clientSocket, buffer, bytesRead, 0) < 0) {
                std::cerr << "Send failed" << std::endl;
                break;
            }
        }
    }
};

void testClient(int port, const std::string& message, int testNum) {
    std::cout << "\\nTest " << testNum << ": Sending message: " << message << std::endl;
    
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Client socket creation failed" << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(port);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Connection failed" << std::endl;
        CLOSE_SOCKET(clientSocket);
        return;
    }

    std::string msgWithNewline = message + "\\n";
    send(clientSocket, msgWithNewline.c_str(), msgWithNewline.length(), 0);

    char buffer[BUFFER_SIZE];
    std::memset(buffer, 0, BUFFER_SIZE);
    int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    
    if (bytesRead > 0) {
        buffer[bytesRead] = '\\0';
        std::cout << "Response: " << buffer;
    }

    CLOSE_SOCKET(clientSocket);
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    std::cout << "Test 1: Starting echo server on port 8080" << std::endl;
    std::thread serverThread([]() { Task96::startServer(8080); });
    serverThread.detach();

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::string testMessages[] = {
        "Hello, Server!",
        "Echo test message",
        "Test 123",
        "Special chars: @#$%",
        "Final test message"
    };

    for (int i = 0; i < 5; i++) {
        testClient(8080, testMessages[i], i + 2);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
