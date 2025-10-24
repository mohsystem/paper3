/*
 * NOTE: This is a minimal, single-threaded, and non-robust implementation for
 * demonstration purposes only. It uses POSIX sockets (Linux/macOS). For Windows,
 * you would need to use Winsock. It does not parse HTTP requests fully.
 *
 * How to compile and run:
 * gcc your_file_name.c -o server
 * ./server
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8082
#define BUFFER_SIZE 2048

void handle_connection(int client_socket);

// Represents the main class for the task
typedef struct {
    // In C, we can use a struct to group related functions or data,
    // though it's not a class in the OOP sense.
    void (*run_server)(void);
} Task130;

void run_c_server() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("C server started on port %d\n", PORT);
    printf("API endpoint available at http://localhost:%d/api/data\n", PORT);

    // --- Test Cases ---
    printf("\n--- How to Test (run from another terminal) ---\n");
    printf("1. Test Preflight (OPTIONS request):\n");
    printf("   curl -v -X OPTIONS http://localhost:%d/api/data -H \"Origin: http://example.com\" -H \"Access-Control-Request-Method: GET\"\n", PORT);
    printf("\n2. Test Actual Request from an allowed origin:\n");
    printf("   curl -v -H \"Origin: http://example.com\" http://localhost:%d/api/data\n", PORT);
    printf("\n3. Test with a different allowed origin (since we use '*'):\n");
    printf("   curl -v -H \"Origin: http://another-site.com\" http://localhost:%d/api/data\n", PORT);
    printf("\n4. Test a non-CORS request (no Origin header):\n");
    printf("   curl -v http://localhost:%d/api/data\n", PORT);
    printf("\n5. Test with a browser. Create an HTML file with the following JS and open it:\n");
    printf("   fetch('http://localhost:%d/api/data').then(r => r.json()).then(console.log).catch(console.error);\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            // continue instead of exit to keep server running
            continue;
        }
        handle_connection(new_socket);
    }
}

void handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE - 1);
    
    char *response = NULL;

    // Simple request routing based on string matching
    if (strncmp(buffer, "OPTIONS /api/data", 17) == 0) {
        // Preflight request
        response = "HTTP/1.1 204 No Content\r\n"
                   "Access-Control-Allow-Origin: *\r\n"
                   "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                   "Access-Control-Allow-Headers: Content-Type\r\n"
                   "Connection: close\r\n"
                   "\r\n";
    } else if (strncmp(buffer, "GET /api/data", 13) == 0) {
        // Actual API request
        char *body = "{\"message\": \"Hello from C API with CORS!\"}";
        int content_length = strlen(body);
        
        // Allocate enough space for headers and body
        char response_buffer[512];
        sprintf(response_buffer,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %d\r\n"
                "Access-Control-Allow-Origin: *\r\n"
                "Connection: close\r\n"
                "\r\n"
                "%s", content_length, body);
        response = response_buffer;
    } else {
        // 404 Not Found
        char *body = "<h1>404 Not Found</h1>";
        int content_length = strlen(body);
        char response_buffer[512];
        sprintf(response_buffer,
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n"
                "Content-Length: %d\r\n"
                "Connection: close\r\n"
                "\r\n"
                "%s", content_length, body);
        response = response_buffer;
    }

    write(client_socket, response, strlen(response));
    close(client_socket);
}


int main() {
    Task130 app;
    app.run_server = run_c_server;
    app.run_server();
    return 0;
}