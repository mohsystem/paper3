
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdint.h>

/* Security: Validate port range to prevent invalid socket configuration */
int isValidPort(int port) {
    return port > 0 && port <= 65535;
}

/* Security: Safe buffer size with maximum limit to prevent excessive memory usage */
#define MAX_BUFFER_SIZE 65536  /* 64KB limit */
#define INITIAL_BUFFER_SIZE 4096

/* Security: Portable secure memset that won't be optimized away */\nvoid secure_memset(void *ptr, int value, size_t num) {\n    if (ptr == NULL || num == 0) {\n        return;\n    }\n    volatile unsigned char *p = (volatile unsigned char *)ptr;\n    while (num--) {\n        *p++ = (unsigned char)value;\n    }\n}\n\n/* Security: Echo server with input validation and bounds checking */\nint runEchoServer(int port) {\n    int server_fd = -1;\n    int client_fd = -1;\n    char *buffer = NULL;\n    int result = -1;\n\n    /* Security: Validate port parameter */\n    if (!isValidPort(port)) {\n        fprintf(stderr, "Error: Invalid port number. Must be 1-65535.\\n");\n        return -1;\n    }\n\n    /* Security: Create socket with error checking */\n    server_fd = socket(AF_INET, SOCK_STREAM, 0);\n    if (server_fd < 0) {\n        fprintf(stderr, "Error: Failed to create socket: %s\\n", strerror(errno));\n        return -1;\n    }\n\n    /* Security: Set SO_REUSEADDR to avoid "Address already in use" errors */\n    int opt = 1;\n    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {\n        fprintf(stderr, "Error: setsockopt failed: %s\\n", strerror(errno));\n        close(server_fd);\n        return -1;\n    }\n\n    /* Security: Initialize address structure with zeros to prevent info leaks */\n    struct sockaddr_in address;\n    memset(&address, 0, sizeof(address));\n    address.sin_family = AF_INET;\n    address.sin_addr.s_addr = INADDR_ANY;\n    address.sin_port = htons((uint16_t)port);\n\n    /* Security: Bind socket with error checking */\n    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {\n        fprintf(stderr, "Error: Bind failed: %s\\n", strerror(errno));\n        close(server_fd);\n        return -1;\n    }\n\n    /* Security: Listen with reasonable backlog */\n    if (listen(server_fd, 5) < 0) {\n        fprintf(stderr, "Error: Listen failed: %s\\n", strerror(errno));\n        close(server_fd);\n        return -1;\n    }\n\n    printf("Server listening on port %d\\n", port);\n\n    /* Security: Accept client connection with validation */\n    struct sockaddr_in client_addr;\n    socklen_t client_len = sizeof(client_addr);\n    memset(&client_addr, 0, sizeof(client_addr));\n    \n    client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);\n    if (client_fd < 0) {\n        fprintf(stderr, "Error: Accept failed: %s\\n", strerror(errno));\n        close(server_fd);\n        return -1;\n    }\n\n    /* Security: Allocate buffer with NULL check */\n    buffer = (char*)calloc(INITIAL_BUFFER_SIZE, sizeof(char));\n    if (buffer == NULL) {\n        fprintf(stderr, "Error: Memory allocation failed\\n");\n        close(client_fd);\n        close(server_fd);\n        return -1;\n    }\n\n    /* Security: Read with size validation and bounds checking */\n    ssize_t bytes_read = recv(client_fd, buffer, INITIAL_BUFFER_SIZE - 1, 0);\n    if (bytes_read < 0) {\n        fprintf(stderr, "Error: Receive failed: %s\\n", strerror(errno));\n        /* Security: Clear sensitive data before freeing */\n        secure_memset(buffer, 0, INITIAL_BUFFER_SIZE);\n        free(buffer);\n        close(client_fd);\n        close(server_fd);\n        return -1;\n    }\n\n    /* Security: Null-terminate received data to prevent buffer overread */\n    buffer[bytes_read] = '\\0';

    printf("Received: %s\\n", buffer);

    /* Security: Echo back with size validation */
    if (bytes_read > 0) {
        ssize_t bytes_sent = send(client_fd, buffer, (size_t)bytes_read, 0);
        if (bytes_sent < 0) {
            fprintf(stderr, "Error: Send failed: %s\\n", strerror(errno));
        } else {
            printf("Echoed %zd bytes back to client\\n", bytes_sent);
        }
    }

    result = 0;

    /* Security: Clear buffer before freeing to prevent data leaks */
    secure_memset(buffer, 0, INITIAL_BUFFER_SIZE);
    free(buffer);
    buffer = NULL;
    
    /* Security: Close sockets in reverse order */
    close(client_fd);
    close(server_fd);

    return result;
}

int main(void) {
    /* Test cases */
    printf("=== Test Case 1: Valid port 8080 ===\\n");
    runEchoServer(8080);

    printf("\\n=== Test Case 2: Valid port 9000 ===\\n");
    runEchoServer(9000);

    printf("\\n=== Test Case 3: Invalid port 0 ===\\n");
    runEchoServer(0);

    printf("\\n=== Test Case 4: Invalid port -1 ===\\n");
    runEchoServer(-1);

    printf("\\n=== Test Case 5: Invalid port 70000 ===\\n");
    runEchoServer(70000);

    return 0;
}
