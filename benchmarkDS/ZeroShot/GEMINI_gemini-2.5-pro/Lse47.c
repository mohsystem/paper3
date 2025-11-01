// To compile: gcc -o Lse47 Lse47.c
// This code is designed for POSIX-compliant systems (Linux, macOS) due to socket and filesystem APIs.
// It will not compile on Windows without modification (e.g., using Winsock).

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <limits.h> // For PATH_MAX
#include <sys/stat.h> // For mkdir
#include <fcntl.h> // For open

#define PORT 8082
#define BUFFER_SIZE 8192
#define IMAGE_DIR "images"

void send_response(int client_fd, const char* status, const char* content_type, const char* body, long body_len) {
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %s\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n\r\n",
             status, content_type, body_len);

    write(client_fd, header, strlen(header));
    if (body && body_len > 0) {
        write(client_fd, body, body_len);
    }
}

void send_file_response(int client_fd, const char* filepath) {
    int file_fd = open(filepath, O_RDONLY);
    if (file_fd == -1) {
        send_response(client_fd, "404 Not Found", "text/plain", "File not found.", 15);
        return;
    }

    // Get file size
    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        send_response(client_fd, "500 Internal Server Error", "text/plain", "Error reading file stats.", 25);
        close(file_fd);
        return;
    }

    // Determine content type (simple version)
    const char* content_type = "application/octet-stream";
    if (strstr(filepath, ".png")) content_type = "image/png";
    else if (strstr(filepath, ".txt")) content_type = "text/plain";
    
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header),
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %ld\r\n"
             "Connection: close\r\n\r\n",
             content_type, file_stat.st_size);
    write(client_fd, header, strlen(header));

    // Send file content
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(file_fd, buffer, sizeof(buffer))) > 0) {
        write(client_fd, buffer, bytes_read);
    }

    close(file_fd);
}

void handle_request(int client_fd) {
    char buffer[BUFFER_SIZE] = {0};
    read(client_fd, buffer, sizeof(buffer) - 1);

    char method[16], path[2048];
    sscanf(buffer, "%s %s", method, path);

    // This block simulates the "decorator" by routing requests based on path.
    if (strcmp(method, "GET") == 0 && strncmp(path, "/images/", 8) == 0) {
        const char* filename = path + 8; // Pointer to start of filename

        // --- Security Check 1: Basic validation ---
        if (strstr(filename, "..") != NULL || strchr(filename, '/') != NULL) {
            send_response(client_fd, "400 Bad Request", "text/plain", "Invalid filename.", 17);
            return;
        }

        char requested_path_str[PATH_MAX];
        snprintf(requested_path_str, sizeof(requested_path_str), "%s/%s", IMAGE_DIR, filename);

        // --- Security Check 2: Canonical path validation ---
        char resolved_base[PATH_MAX];
        char resolved_req[PATH_MAX];

        if (realpath(IMAGE_DIR, resolved_base) == NULL) {
            send_response(client_fd, "500 Internal Server Error", "text/plain", "Server misconfiguration.", 24);
            return;
        }

        if (realpath(requested_path_str, resolved_req) == NULL) {
            // This usually means the file doesn't exist.
            send_response(client_fd, "404 Not Found", "text/plain", "File not found.", 15);
            return;
        }
        
        // Ensure the resolved path of the request is within the base directory.
        if (strncmp(resolved_base, resolved_req, strlen(resolved_base)) != 0) {
            send_response(client_fd, "403 Forbidden", "text/plain", "Access denied.", 14);
            return;
        }

        // If all checks pass, send the file
        send_file_response(client_fd, resolved_req);

    } else {
        send_response(client_fd, "404 Not Found", "text/plain", "Not Found.", 10);
    }
}

void setup_test_files() {
    mkdir(IMAGE_DIR, 0755);
    FILE* png = fopen("images/test1.png", "wb");
    if (png) {
        fwrite("\x89PNG\r\n\x1a\n", 1, 8, png);
        fclose(png);
    }
    FILE* txt = fopen("images/safe.txt", "w");
    if (txt) {
        fputs("This is a safe file.", txt);
        fclose(txt);
    }
}

int main() {
    setup_test_files();
    
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

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

    printf("Server started on port %d\n", PORT);
    printf("--- Test Cases (run in another terminal) ---\n");
    printf("1) Access a valid file: curl http://localhost:%d/images/test1.png --output test1_out.png\n", PORT);
    printf("2) Access another valid file: curl http://localhost:%d/images/safe.txt\n", PORT);
    printf("3) Access a non-existent file: curl -i http://localhost:%d/images/nonexistent.jpg\n", PORT);
    printf("4) Attempt path traversal (should fail): curl -i http://localhost:%d/images/../Lse47.c\n", PORT);
    printf("5) Attempt another path traversal: curl -i http://localhost:%d/images/..%%2fLse47.c\n", PORT);

    while (1) {
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue; // Continue to next connection
        }
        handle_request(client_fd);
        close(client_fd);
    }
    return 0;
}