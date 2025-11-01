#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function that accepts input and returns output via dynamic string (caller frees)
char* greet(const char* name) {
    const char* prefix = "Hello, ";
    const char* suffix = "!";
    size_t len = strlen(prefix) + strlen(name) + strlen(suffix);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    strcpy(out, prefix);
    strcat(out, name);
    strcat(out, suffix);
    return out;
}

// Minimal HTTP server in C; returns 0 on success
int start_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    printf("C HTTP server running on http://127.0.0.1:%d/\n", port);

    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            break;
        }

        char buffer[4096];
        ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
        if (n <= 0) {
            close(client_fd);
            continue;
        }
        buffer[n] = '\0';

        // Parse request line
        char method[16], path[1024], version[16];
        method[0] = path[0] = version[0] = '\0';
        sscanf(buffer, "%15s %1023s %15s", method, path, version);

        char* body = NULL;
        int status_ok = 0;
        if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
            body = greet("C User");
            status_ok = 1;
        } else {
            body = strdup("Not Found");
        }
        if (!body) body = strdup("Internal Server Error");

        char header[256];
        const char* status_line = status_ok ? "HTTP/1.1 200 OK\r\n" : "HTTP/1.1 404 Not Found\r\n";
        snprintf(header, sizeof(header),
                 "%sContent-Type: text/plain; charset=UTF-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                 status_line, strlen(body));

        write(client_fd, header, strlen(header));
        write(client_fd, body, strlen(body));

        free(body);
        close(client_fd);
    }

    close(server_fd);
    return 0;
}

int main(void) {
    // 5 test cases for greet
    char* t1 = greet("Alice");
    char* t2 = greet("Bob");
    char* t3 = greet("Charlie");
    char* t4 = greet("Dana");
    char* t5 = greet("Eve");
    if (t1) { printf("%s\n", t1); free(t1); }
    if (t2) { printf("%s\n", t2); free(t2); }
    if (t3) { printf("%s\n", t3); free(t3); }
    if (t4) { printf("%s\n", t4); free(t4); }
    if (t5) { printf("%s\n", t5); free(t5); }

    // Start server on port 8082
    start_server(8082);
    return 0;
}