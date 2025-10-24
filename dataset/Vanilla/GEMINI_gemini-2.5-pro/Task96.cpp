#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <mutex>

// Platform-specific socket includes and setup
#if defined(_WIN32) || defined(_WIN64)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
    using socket_t = SOCKET;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #define CLOSE_SOCKET close
    using socket_t = int;
    const int INVALID_SOCKET = -1;
#endif

void handle_client(socket_t clientSocket) {
    char clientIp[INET_ADDRSTRLEN];
    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    getpeername(clientSocket, (struct sockaddr*)&client_addr, &client_addr_size);
    inet_ntop(AF_INET, &client_addr.sin_addr, clientIp, INET_ADDRSTRLEN);

    std::cout << "Client connected: " << clientIp << std::endl;

    char buffer[4096];
    int bytesReceived;

    // Echo messages back to client
    while ((bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytesReceived] = '\0'; // Null-terminate for printing
        std::cout << "Received from " << clientIp << ": " << buffer;
        send(clientSocket, buffer, bytesReceived, 0);
    }

    if (bytesReceived == 0) {
        std::cout << "Client " << clientIp << " disconnected gracefully." << std::endl;
    } else {
        #if defined(_WIN32) || defined(_WIN64)
            std::cerr << "recv failed with error: " << WSAGetLastError() << std::endl;
        #else
            perror("recv failed");
        #endif
    }

    CLOSE_SOCKET(clientSocket);
}

void start_server(int port) {
#if defined(_WIN32) || defined(_WIN64)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        return;
    }
#endif

    socket_t serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Bind failed." << std::endl;
        CLOSE_SOCKET(serverSocket);
        return;
    }

    if (listen(serverSocket, 5) < 0) {
        std::cerr << "Listen failed." << std::endl;
        CLOSE_SOCKET(serverSocket);
        return;
    }

    std::cout << "Server started on port: " << port << std::endl;
    std::cout << "Waiting for client connections..." << std::endl;

    while (true) {
        socket_t clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue; // Continue to the next iteration to accept new connections
        }
        
        // Create a new thread to handle the client
        std::thread clientThread(handle_client, clientSocket);
        clientThread.detach(); // Detach the thread to run independently
    }

    CLOSE_SOCKET(serverSocket);
#if defined(_WIN32) || defined(_WIN64)
    WSACleanup();
#endif
}

class Task96 {
public:
    static void main() {
        // --- Test Cases ---
        // The server will run indefinitely. To test, you need a client program
        // or a tool like `telnet` or `netcat`.
        //
        // How to test (5 test cases):
        // 1. Compile and run this C++ program.
        // 2. Open a new terminal/command prompt.
        // 3. Test Case 1: Connect using netcat: `nc 127.0.0.1 12345` or telnet: `telnet 127.0.0.1 12345`
        // 4. Test Case 2: Type "Hello C++ Server" and press Enter. The server should echo it back.
        // 5. Test Case 3: Type "This is a multithreaded test" and press Enter.
        // 6. Test Case 4: Open another new terminal and connect again.
        //    Send a message from this new client. The server handles multiple clients concurrently.
        // 7. Test Case 5: Close one of the nc/telnet sessions (Ctrl+C or Ctrl+D).
        //    The other session should remain active.

        int port = 12345;
        start_server(port);
    }
};

int main() {
    Task96::main();
    return 0;
}