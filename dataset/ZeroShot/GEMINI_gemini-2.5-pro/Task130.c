#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Platform-specific includes for networking
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define close closesocket
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#endif

void handle_client(SOCKET client_socket) {
    char buffer[2048] = {0};
    char response[2048] = {0};

#ifdef _WIN32
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
#else
    ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);
#endif

    if (bytes_received <= 0) {
        close(client_socket);
        return;
    }

    // For security, restrict the origin in a production environment.
    const char* cors_headers =
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Connection: close\r\n";

    // Simple, insecure request routing for demonstration.
    if (strncmp(buffer, "GET /api/data", 13) == 0) {
        const char* body = "{\"message\": \"Hello from a CORS-enabled C API!\"}";
        snprintf(response, sizeof(response),
                 "HTTP/1.1 200 OK\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %zu\r\n"
                 "%s\r\n%s",
                 strlen(body), cors_headers, body);
    } else if (strncmp(buffer, "OPTIONS /api/data", 17) == 0) {
        snprintf(response, sizeof(response),
                 "HTTP/1.1 204 No Content\r\n"
                 "%s\r\n",
                 cors_headers);
    } else if (strncmp(buffer, "POST /api/data", 14) == 0 || strncmp(buffer, "PUT /api/data", 13) == 0) {
        const char* body = "{\"error\": \"405 Method Not Allowed\"}";
        snprintf(response, sizeof(response),
                 "HTTP/1.1 405 Method Not Allowed\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %zu\r\n"
                 "%s\r\n%s",
                 strlen(body), cors_headers, body);
    } else {
        const char* body = "{\"error\": \"404 Not Found\"}";
        snprintf(response, sizeof(response),
                 "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: application/json\r\n"
                 "Content-Length: %zu\r\n"
                 "%s\r\n%s",
                 strlen(body), cors_headers, body);
    }

    send(client_socket, response, strlen(response), 0);
    close(client_socket);
}

void run_tests(int port) {
    printf("\n--- How to Test ---\n");
    printf("Run the following cURL commands in a separate terminal:\n");
    printf("1. Test Preflight (OPTIONS) request for CORS:\n");
    printf("   curl -i -X OPTIONS http://localhost:%d/api/data -H \"Origin: http://example.com\" -H \"Access-Control-Request-Method: GET\"\n", port);
    printf("\n2. Test actual (GET) request from a simulated foreign origin:\n");
    printf("   curl -i -X GET http://localhost:%d/api/data -H \"Origin: http://example.com\"\n", port);
    printf("\n3. Test GET request without an Origin header (like a server-to-server call):\n");
    printf("   curl -i http://localhost:%d/api/data\n", port);
    printf("\n4. Test a non-existent API endpoint:\n");
    printf("   curl -i http://localhost:%d/api/other\n", port);
    printf("\n5. Test a disallowed method (e.g., POST) on the API endpoint:\n");
    printf("   curl -i -X POST http://localhost:%d/api/data -H \"Origin: http://example.com\"\n", port);
    printf("--------------------\n\n");
}

int main() {
    int port = 8080;
    run_tests(port);
    
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n"); return 1;
    }
#endif

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n"); return 1;
    }
    
    int opt = 1;
#ifdef _WIN32
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
#else
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
#endif

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed.\n");
        close(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    if (listen(server_socket, 10) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed.\n");
        close(server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    printf("C server started on port %d...\n", port);
    printf("NOTE: This is a minimal, non-production server for demonstration.\n");

    while (1) {
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed.\n"); continue;
        }
        handle_client(client_socket); // In production, this should be in a new thread.
    }

    close(server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}