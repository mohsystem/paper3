/*
 * NOTE: This is a minimal, non-production-ready HTTP server for demonstration.
 * It lacks many features and security hardening of a real server.
 * This implementation is for POSIX-compliant systems (Linux, macOS).
 *
 * COMPILE with: gcc -o task130 task130.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8083
#define BUFFER_SIZE 2048

// Whitelist of allowed origins for CORS
const char* ALLOWED_ORIGINS[] = {
    "http://localhost:3000",
    "https://safe.example.com",
    NULL // Sentinel value
};

// Securely checks if an origin is in the whitelist
int is_origin_allowed(const char* origin) {
    if (origin == NULL) {
        return 0;
    }
    for (int i = 0; ALLOWED_ORIGINS[i] != NULL; i++) {
        if (strcmp(origin, ALLOWED_ORIGINS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// A very basic function to extract a header value from the request buffer
char* get_header_value(const char* request, const char* header_name) {
    char* header_line = strstr(request, header_name);
    if (header_line == NULL) {
        return NULL;
    }

    char* value_start = header_line + strlen(header_name);
    while (*value_start == ' ' || *value_start == ':') {
        value_start++;
    }

    char* value_end = strstr(value_start, "\r\n");
    if (value_end == NULL) {
        return NULL;
    }

    size_t value_len = value_end - value_start;
    char* value = (char*)malloc(value_len + 1);
    if (value == NULL) return NULL;

    strncpy(value, value_start, value_len);
    value[value_len] = '\0';
    return value;
}

void handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    long valread = read(client_socket, buffer, BUFFER_SIZE - 1);
    if (valread < 0) {
        perror("read failed");
        close(client_socket);
        return;
    }
    
    // Extract method and path
    char method[16], path[256];
    sscanf(buffer, "%15s %255s", method, path);

    // Get Origin header
    char* origin = get_header_value(buffer, "Origin");

    char response[BUFFER_SIZE];
    
    // Handle API route /api/data
    if (strcmp(path, "/api/data") == 0) {
        if (strcmp(method, "OPTIONS") == 0) {
            // Preflight request
            if (is_origin_allowed(origin)) {
                snprintf(response, BUFFER_SIZE,
                    "HTTP/1.1 204 No Content\r\n"
                    "Access-Control-Allow-Origin: %s\r\n"
                    "Access-Control-Allow-Methods: GET, OPTIONS\r\n"
                    "Access-Control-Allow-Headers: Content-Type, X-Requested-With\r\n"
                    "\r\n", origin);
            } else {
                 strcpy(response, "HTTP/1.1 204 No Content\r\n\r\n");
            }
        } else if (strcmp(method, "GET") == 0) {
            // Actual API request
            const char* json_body = "{\"message\": \"Hello from C CORS API!\"}";
            if (is_origin_allowed(origin)) {
                snprintf(response, BUFFER_SIZE,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: %zu\r\n"
                    "Access-Control-Allow-Origin: %s\r\n"
                    "\r\n"
                    "%s", strlen(json_body), origin, json_body);
            } else {
                 snprintf(response, BUFFER_SIZE,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/json\r\n"
                    "Content-Length: %zu\r\n"
                    "\r\n"
                    "%s", strlen(json_body), json_body);
            }
        } else {
             strcpy(response, "HTTP/1.1 405 Method Not Allowed\r\n\r\n");
        }
    } else {
        // Not Found
        strcpy(response, "HTTP/1.1 404 Not Found\r\nContent-Length: 9\r\n\r\nNot Found");
    }

    write(client_socket, response, strlen(response));
    
    if (origin != NULL) {
        free(origin);
    }
    close(client_socket);
}

int main() {
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
    if (listen(server_fd, 10) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("C server started on port %d\n", PORT);
    printf("Run the test cases below in a new terminal.\n");
    printf("\n--- Test Cases (run with curl) ---\n");
    printf("1. Preflight OPTIONS from allowed origin (should succeed with CORS headers):\n"
           "   curl -v -X OPTIONS http://localhost:8083/api/data -H \"Origin: https://safe.example.com\" -H \"Access-Control-Request-Method: GET\"\n");
    printf("\n2. GET from allowed origin (should succeed with data and CORS header):\n"
           "   curl -v http://localhost:8083/api/data -H \"Origin: https://safe.example.com\"\n");
    printf("\n3. GET from disallowed origin (should get data but NO CORS header):\n"
           "   curl -v http://localhost:8083/api/data -H \"Origin: http://evil.com\"\n");
    printf("\n4. GET with no Origin header (should get data but NO CORS header):\n"
           "   curl -v http://localhost:8083/api/data\n");
    printf("\n5. GET request to a non-existent path (should get 404):\n"
           "   curl -v http://localhost:8083/other/path -H \"Origin: https://safe.example.com\"\n");

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to next iteration
        }
        handle_connection(new_socket);
    }
    return 0;
}