#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000
#define BUFFER_SIZE 2048

// A simple URL decoder function
void url_decode(const char* src, char* dst) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a'-'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a'-'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16*a + b;
            src+=3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

// Function to handle a single client connection
void handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_socket, buffer, BUFFER_SIZE - 1);

    const char* payload_param = strstr(buffer, "payload=");
    
    char* response_body;
    char* response_header;
    
    if (payload_param == NULL) {
        response_body = "Error: 'payload' parameter is missing.";
        response_header = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n";
    } else {
        char encoded_payload[BUFFER_SIZE];
        char decoded_payload[BUFFER_SIZE];
        
        // Extract the payload value
        sscanf(payload_param, "payload=%s", encoded_payload);
        // The value might end with ' HTTP/1.1', so we cut it there.
        char* http_ver = strstr(encoded_payload, " HTTP/");
        if (http_ver) {
            *http_ver = '\0';
        }
        
        url_decode(encoded_payload, decoded_payload);

        // Instead of a full YAML parser, we do a simple string search.
        if (strstr(decoded_payload, "type: Create") != NULL) {
            response_body = "Error: Payload with type 'Create' is not allowed.";
            response_header = "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n";
        } else {
            response_body = "OK: Payload processed successfully.";
            response_header = "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n";
        }
    }
    
    char http_response[BUFFER_SIZE];
    snprintf(http_response, sizeof(http_response), "%sContent-Length: %zu\r\n\r\n%s", 
             response_header, strlen(response_body), response_body);
             
    write(client_socket, http_response, strlen(http_response));
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
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("C server started on port %d\n", PORT);
    // --- 5 Test Cases (run in a separate terminal) ---
    printf("\n--- Test Cases ---\n");
    printf("1. No payload parameter (expects error):\n");
    printf("   curl \"http://localhost:5000/\"\n");
    printf("\n2. Payload with 'type: Create' (expects error):\n");
    printf("   curl \"http://localhost:5000/?payload=type%%3A%%20Create\"\n");
    printf("\n3. Payload with different type (expects OK):\n");
    printf("   curl \"http://localhost:5000/?payload=type%%3A%%20Update\"\n");
    printf("\n4. Payload with 'type: Create' and other data (expects error):\n");
    printf("   curl \"http://localhost:5000/?payload=name%%3A%%20my-app%%0Atype%%3A%%20Create\"\n");
    printf("\n5. Valid YAML payload without 'type' key (expects OK):\n");
    printf("   curl \"http://localhost:5000/?payload=name%%3A%%20my-app\"\n");
    printf("\n--- Server is running. Press Ctrl+C to stop. ---\n");

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            // continue to next iteration instead of exiting
            continue; 
        }
        handle_connection(new_socket);
    }
    
    return 0;
}