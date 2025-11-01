#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// 4) Function that is called when the user visits the specified URL
static const char* index_handler(void) {
    // 5) Returns the message to the user
    return "Hello, secure world.";
}

static void send_response(int client_fd, int code, const char* status, const char* body) {
    char header[256];
    size_t body_len = strlen(body);
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/plain; charset=utf-8\r\n"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n"
                     "\r\n",
                     code, status, body_len);
    if (n < 0) return;
    size_t total = 0;
    while (total < (size_t)n) {
        ssize_t w = send(client_fd, header + total, (size_t)n - total, 0);
        if (w <= 0) break;
        total += (size_t)w;
    }
    total = 0;
    while (total < body_len) {
        ssize_t w = send(client_fd, body + total, body_len - total, 0);
        if (w <= 0) break;
        total += (size_t)w;
    }
}

static int create_server_socket(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        perror("setsockopt");
        close(fd);
        return -1;
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) != 1) {
        perror("inet_pton");
        close(fd);
        return -1;
    }
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("bind");
        close(fd);
        return -1;
    }
    if (listen(fd, 8) != 0) {
        perror("listen");
        close(fd);
        return -1;
    }
    return fd;
}

static void run_server(void) {
    // 2) Creates the application object (server socket)
    int server_fd = create_server_socket(8082);
    if (server_fd < 0) {
        fprintf(stderr, "Server initialization failed.\n");
        return;
    }
    printf("Server running at http://127.0.0.1:8082/\n");

    // 6) Runs the application (accept loop)
    for (;;) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        char buf[4096];
        ssize_t n = recv(client_fd, buf, sizeof(buf), 0);
        if (n <= 0) {
            close(client_fd);
            continue;
        }
        // Very basic request line parsing
        size_t line_end = 0;
        for (size_t i = 0; i < (size_t)n; i++) {
            if (i + 1 < (size_t)n && buf[i] == '\r' && buf[i + 1] == '\n') {
                line_end = i;
                break;
            }
        }
        if (line_end == 0) {
            send_response(client_fd, 400, "Bad Request", "Bad Request");
            close(client_fd);
            continue;
        }

        char method[8] = {0};
        char path[2049] = {0};
        char version[16] = {0};
        // Safe parsing with width limits
        int scanned = sscanf(buf, "%7s %2048s %15s", method, path, version);
        if (scanned != 3) {
            send_response(client_fd, 400, "Bad Request", "Bad Request");
            close(client_fd);
            continue;
        }
        if (strcmp(method, "GET") != 0) {
            send_response(client_fd, 405, "Method Not Allowed", "Method Not Allowed");
            close(client_fd);
            continue;
        }

        // 3) "Decorator": Map URL to function (dispatch)
        if (strcmp(path, "/") == 0) {
            // 4 & 5) Call the function and return the message
            send_response(client_fd, 200, "OK", index_handler());
        } else {
            send_response(client_fd, 404, "Not Found", "Not Found");
        }

        close(client_fd);
    }
}

int main(void) {
    // Five test cases for index_handler()
    const char* expected = "Hello, secure world.";
    bool all_pass = true;
    for (int i = 0; i < 5; i++) {
        const char* res = index_handler();
        if (strcmp(res, expected) != 0) {
            all_pass = false;
        }
    }
    printf("%s\n", all_pass ? "All tests passed." : "Some tests failed.");

    // 1) "Import" the web application class (in C this is conceptual; includes are at top)

    run_server();
    return 0;
}