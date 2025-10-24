#include <iostream>
#include <string>
#include <cstring>
#include <vector>

// Platform-specific includes for networking
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#endif

// Function to handle a single client connection
void handle_client(SOCKET client_socket) {
    char buffer[2048] = {0};

#ifdef _WIN32
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
#else
    ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);
#endif

    if (bytes_received <= 0) {
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
        return;
    }

    std::string request(buffer);
    std::string response;
    std::string headers;
    std::string body;

    // For security, restrict the origin in a production environment.
    // E.g., "Access-Control-Allow-Origin: https://your-frontend-domain.com\r\n"
    std::string cors_headers =
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Connection: close\r\n"; // Ensure connection is closed after response

    // Simple, insecure request routing for demonstration.
    if (request.rfind("GET /api/data", 0) == 0) {
        body = "{\"message\": \"Hello from a CORS-enabled C++ API!\"}";
        headers = "HTTP/1.1 200 OK\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: " + std::to_string(body.length()) + "\r\n" +
                  cors_headers + "\r\n";
        response = headers + body;
    } else if (request.rfind("OPTIONS /api/data", 0) == 0) {
        headers = "HTTP/1.1 204 No Content\r\n" + cors_headers + "\r\n";
        response = headers;
    } else if (request.rfind("POST /api/data", 0) == 0 || request.rfind("PUT /api/data", 0) == 0) {
        body = "{\"error\": \"405 Method Not Allowed\"}";
        headers = "HTTP/1.1 405 Method Not Allowed\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: " + std::to_string(body.length()) + "\r\n" +
                  cors_headers + "\r\n";
        response = headers + body;
    } else {
        body = "{\"error\": \"404 Not Found\"}";
        headers = "HTTP/1.1 404 Not Found\r\n"
                  "Content-Type: application/json\r\n"
                  "Content-Length: " + std::to_string(body.length()) + "\r\n" +
                  cors_headers + "\r\n"; // Send CORS headers even on errors
        response = headers + body;
    }

#ifdef _WIN32
    send(client_socket, response.c_str(), (int)response.length(), 0);
    closesocket(client_socket);
#else
    send(client_socket, response.c_str(), response.length(), 0);
    close(client_socket);
#endif
}

void run_tests(int port) {
    std::cout << "\n--- How to Test ---" << std::endl;
    std::cout << "Run the following cURL commands in a separate terminal:" << std::endl;
    std::cout << "1. Test Preflight (OPTIONS) request for CORS:" << std::endl;
    std::cout << "   curl -i -X OPTIONS http://localhost:" << port << "/api/data -H \"Origin: http://example.com\" -H \"Access-Control-Request-Method: GET\"" << std::endl;
    std::cout << "\n2. Test actual (GET) request from a simulated foreign origin:" << std::endl;
    std::cout << "   curl -i -X GET http://localhost:" << port << "/api/data -H \"Origin: http://example.com\"" << std::endl;
    std::cout << "\n3. Test GET request without an Origin header (like a server-to-server call):" << std::endl;
    std::cout << "   curl -i http://localhost:" << port << "/api/data" << std::endl;
    std::cout << "\n4. Test a non-existent API endpoint:" << std::endl;
    std::cout << "   curl -i http://localhost:" << port << "/api/other" << std::endl;
    std::cout << "\n5. Test a disallowed method (e.g., POST) on the API endpoint:" << std::endl;
    std::cout << "   curl -i -X POST http://localhost:" << port << "/api/data -H \"Origin: http://example.com\"" << std::endl;
    std::cout << "--------------------\n" << std::endl;
}

int main() {
    int port = 8080;
    run_tests(port);

#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl; return 1;
    }
#endif

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl; return 1;
    }

    int opt = 1;
#ifdef _WIN32
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
#ifdef _WIN32
        closesocket(server_socket); WSACleanup();
#else
        close(server_socket);
#endif
        return 1;
    }

    if (listen(server_socket, 10) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
#ifdef _WIN32
        closesocket(server_socket); WSACleanup();
#else
        close(server_socket);
#endif
        return 1;
    }
    
    std::cout << "C++ server started on port " << port << "..." << std::endl;
    std::cout << "NOTE: This is a minimal, non-production server for demonstration." << std::endl;

    while (true) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl; continue;
        }
        handle_client(client_socket); // In production, this should be in a new thread.
    }
    
#ifdef _WIN32
    closesocket(server_socket); WSACleanup();
#else
    close(server_socket);
#endif
    return 0;
}