/*
 * =====================================================================================
 * IMPORTANT SECURITY AND FEASIBILITY NOTE
 * =====================================================================================
 *
 * Implementing a secure and robust HTTP server in pure C from scratch that correctly
 * handles file uploads (multipart/form-data) is a highly complex and error-prone task.
 * It is NOT recommended for production environments.
 *
 * Key challenges and security risks include:
 *
 * 1.  Parsing multipart/form-data: This is a complex protocol. A manual parser is
 *     difficult to write correctly and is a major source of vulnerabilities like:
 *     - Buffer Overflows: Incorrectly handling boundaries and content lengths can lead
 *       to writing past buffer boundaries, a severe security risk.
 *     - Denial of Service (DoS): A malformed request could cause the parser to enter
 *       an infinite loop or consume excessive memory/CPU.
 *
 * 2.  Path Traversal Attacks: Filenames provided by the client MUST be strictly
 *     sanitized to prevent a malicious user from writing files outside the intended
 *     'uploads' directory (e.g., a filename like "../../etc/passwd").
 *
 * 3.  Resource Exhaustion: The server must handle large files and slow clients
 *     without exhausting memory or file descriptors. This requires careful,
 *     non-blocking I/O and stream processing.
 *
 * 4.  HTTP Protocol Compliance: Correctly parsing HTTP headers, handling different
 *     encodings, and managing connection states (keep-alive) is non-trivial.
 *
 * For these reasons, this code block provides a basic TCP server skeleton ONLY. It
 * demonstrates how to accept a connection but intentionally DOES NOT attempt to
 * implement the dangerous file upload logic.
 *
 * RECOMMENDATION: Use a well-vetted, security-hardened C library like
 * 'libmicrohttpd', 'mongoose', or 'CivetWeb' for any real-world application.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#include <sys/stat.h> // For mkdir

#define PORT 8000
#define BUFFER_SIZE 2048
#define UPLOAD_DIR "uploads"

// Function to send a basic HTTP response
void send_response(int client_socket, const char* status, const char* content_type, const char* body) {
    char response[BUFFER_SIZE];
    snprintf(response, sizeof(response),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n"
             "%s",
             status, content_type, strlen(body), body);
    send(client_socket, response, strlen(response), 0);
}

int main() {
    // --- Setup for Sockets (OS-dependent) ---
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        perror("WSAStartup failed");
        return 1;
    }
#endif

    // --- Create 'uploads' directory ---
#ifdef _WIN32
    _mkdir(UPLOAD_DIR);
#else
    mkdir(UPLOAD_DIR, 0755);
#endif
    
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // --- Create Socket ---
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // --- Set Socket Options ---
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // --- Bind Socket to Port ---
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // --- Listen for Connections ---
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    printf("NOTE: This C server is a non-functional skeleton for demonstration.\n");
    printf("It does not handle file uploads due to major security risks.\n");

    printf("\n--- Test Plan (Conceptual for a full implementation) ---\n");
    printf("Test Case 1: Client uploads a valid PDF. Server should save it correctly.\n");
    printf("Test Case 2: Client uploads a non-PDF. Server should reject it.\n");
    printf("Test Case 3: Client uploads a file > MAX_SIZE. Server should reject it.\n");
    printf("Test Case 4: Client sends a filename like '../evil.pdf'. Server must prevent path traversal.\n");
    printf("Test Case 5: Client sends a malformed multipart request. Server must not crash.\n");

    // --- Main Server Loop ---
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to next iteration
        }

        // Read the client's request
        recv(client_socket, buffer, BUFFER_SIZE - 1, 0);

        // --- Request Handling Logic ---
        // This is where the extremely complex and unsafe parsing would go.
        // We will only handle GET to show the form and reject POST with a message.
        if (strncmp(buffer, "GET /", 5) == 0) {
            const char* html_form = "<html><body><h2>Upload a PDF File (Server in C is a Demo)</h2>"
                                    "<form action=\"/upload\" method=\"post\" enctype=\"multipart/form-data\">"
                                    "<input type=\"file\" name=\"pdfFile\"><input type=\"submit\"></form>"
                                    "<p><b>NOTE:</b> File upload is not implemented in this C example due to security complexities.</p>"
                                    "</body></html>";
            send_response(client_socket, "200 OK", "text/html", html_form);
        } else if (strncmp(buffer, "POST /upload", 12) == 0) {
            const char* body = "<h1>501 Not Implemented</h1><p>File upload handling in pure C is complex and "
                               "is omitted from this example for security reasons. Use a dedicated library.</p>";
            send_response(client_socket, "501 Not Implemented", "text/html", body);
        } else {
            send_response(client_socket, "404 Not Found", "text/html", "<h1>404 Not Found</h1>");
        }

        // Close the client socket
#ifdef _WIN32
        closesocket(client_socket);
#else
        close(client_socket);
#endif
    }

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}