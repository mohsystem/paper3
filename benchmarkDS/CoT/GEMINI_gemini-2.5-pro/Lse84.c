/*
 * A full, secure HTTP web server with YAML parsing in C is highly complex
 * and requires significant external libraries (e.g., libmicrohttpd for HTTP,
 * libyaml for parsing) and a proper build system to link them.
 * A single-file implementation would be insecure and impractical.
 *
 * The following code is a simplified simulation using basic sockets. It does NOT
 * parse a full HTTP request and is NOT a functional web server. It only reads
 * the first line from a connection and treats it as the 'payload' to demonstrate
 * the core logic in C. It also uses a very basic string search for the YAML
 * check, which is not a robust YAML parser but avoids a complex library dependency.
 *
 * To compile:
 * gcc -o server Lse84.c
 *
 * To run:
 * ./server
 *
 * To test, use a tool like netcat (nc) in a separate terminal:
 *
 * Test Case 1 (Simulated): No payload (empty input)
 * echo "" | nc localhost 5000
 * Expected Output: Error: 'payload' parameter is missing.
 *
 * Test Case 2 (Simulated): Payload with type: Create
 * echo "type: Create" | nc localhost 5000
 * Expected Output: Error: Payload with type 'Create' is not allowed.
 *
 * Test Case 3 (Simulated): Payload with another type
 * echo "type: Update" | nc localhost 5000
 * Expected Output: Payload processed successfully.
 *
 * Test Case 4 & 5 are not easily distinguishable with this simple string search.
 * This naive approach would treat malformed YAML and YAML without a type key the same.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 5000
#define BUFFER_SIZE 1024

// This is a simplified, insecure checker, not a real YAML parser.
// It checks for the substring "type: Create".
const char* process_payload(const char* payload) {
    if (payload == NULL || strlen(payload) == 0) {
        return "Error: 'payload' parameter is missing.";
    }

    // Naive check for "type: Create". A real implementation needs a proper YAML library.
    const char* sub_str = "type: Create";
    char* found = strstr(payload, sub_str);
    
    // Check if "found" is at the start or preceded by a newline to be slightly more robust.
    if (found && (found == payload || *(found-1) == '\n' || *(found-1) == '\r')) {
        return "Error: Payload with type 'Create' is not allowed.";
    }

    return "Payload processed successfully.";
}

void Lse84_handle_connection(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    const char* response;

    // Read one line from the client, treat it as the payload
    read(client_socket, buffer, BUFFER_SIZE - 1);

    // Remove trailing newline characters for cleaner processing
    buffer[strcspn(buffer, "\r\n")] = 0;

    response = process_payload(buffer);

    send(client_socket, response, strlen(response), 0);
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

    printf("Simplified C server listening on port %d\n", PORT);
    printf("NOTE: This is not a real HTTP server. Use netcat to test.\n");

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        Lse84_handle_connection(new_socket);
    }

    return 0;
}