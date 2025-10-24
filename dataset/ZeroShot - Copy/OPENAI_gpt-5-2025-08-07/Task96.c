#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 8192

typedef struct {
    int server_fd;
    unsigned short port;
    int running;
} EchoServer;

static void set_socket_timeouts(int fd, int seconds) {
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

static void handle_client(int cfd) {
    set_socket_timeouts(cfd, 5);
    char *buffer = (char*)malloc(MAX_LEN);
    if (!buffer) {
        close(cfd);
        return;
    }
    size_t total = 0;
    char temp[512];
    int done = 0;
    while (!done && total < MAX_LEN) {
        ssize_t n = recv(cfd, temp, sizeof(temp), 0);
        if (n <= 0) break;
        for (ssize_t i = 0; i < n; ++i) {
            if (temp[i] == '\n') {
                size_t toCopy = (size_t)i;
                if (total + toCopy > MAX_LEN) toCopy = MAX_LEN - total;
                memcpy(buffer + total, temp, toCopy);
                total += toCopy;
                done = 1;
                break;
            }
        }
        if (!done) {
            size_t toCopy = (size_t)n;
            if (total + toCopy > MAX_LEN) toCopy = MAX_LEN - total;
            memcpy(buffer + total, temp, toCopy);
            total += toCopy;
        }
    }
    size_t sent = 0;
    while (sent < total) {
        ssize_t m = send(cfd, buffer + sent, total - sent, 0);
        if (m <= 0) break;
        sent += (size_t)m;
    }
    shutdown(cfd, SHUT_RDWR);
    close(cfd);
    free(buffer);
}

static int start_server(EchoServer* srv, unsigned short port) {
    if (!srv) return -1;
    srv->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (srv->server_fd < 0) return -1;

    int yes = 1;
    setsockopt(srv->server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    if (bind(srv->server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(srv->server_fd);
        return -1;
    }
    if (listen(srv->server_fd, 50) < 0) {
        close(srv->server_fd);
        return -1;
    }

    socklen_t len = sizeof(addr);
    if (getsockname(srv->server_fd, (struct sockaddr*)&addr, &len) == 0) {
        srv->port = ntohs(addr.sin_port);
    } else {
        srv->port = port;
    }
    srv->running = 1;
    return 0;
}

static void stop_server(EchoServer* srv) {
    if (!srv) return;
    srv->running = 0;
    shutdown(srv->server_fd, SHUT_RDWR);
    close(srv->server_fd);
}

static char* echo_client(const char* host, unsigned short port, const char* message) {
    if (!host || !message || port == 0) return NULL;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return NULL;

    set_socket_timeouts(fd, 5);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return NULL;
    }
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return NULL;
    }

    size_t msg_len = strlen(message);
    if (msg_len > MAX_LEN) msg_len = MAX_LEN;
    size_t send_len = msg_len + 1;
    char* sendbuf = (char*)malloc(send_len);
    if (!sendbuf) {
        close(fd);
        return NULL;
    }
    memcpy(sendbuf, message, msg_len);
    sendbuf[msg_len] = '\n';

    size_t sent = 0;
    while (sent < send_len) {
        ssize_t n = send(fd, sendbuf + sent, send_len - sent, 0);
        if (n <= 0) break;
        sent += (size_t)n;
    }
    shutdown(fd, SHUT_WR);
    free(sendbuf);

    char* out = (char*)malloc(MAX_LEN + 1);
    if (!out) {
        close(fd);
        return NULL;
    }
    size_t total = 0;
    char buf[512];
    ssize_t n;
    while ((n = recv(fd, buf, sizeof(buf), 0)) > 0) {
        size_t toCopy = (size_t)n;
        if (total + toCopy > MAX_LEN) toCopy = MAX_LEN - total;
        memcpy(out + total, buf, toCopy);
        total += toCopy;
        if (total >= MAX_LEN) break;
    }
    out[total] = '\0';
    close(fd);
    return out;
}

int main(void) {
    EchoServer server;
    if (start_server(&server, 0) != 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    const char* tests[5] = {
        "hello",
        "test message",
        "",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
        "Line1\\nLine2 with symbols !@#$%^&*()_+"
    };

    for (int i = 0; i < 5; ++i) {
        char* resp = echo_client("127.0.0.1", server.port, tests[i]);
        if (resp) {
            printf("Case %d: %s\n", i + 1, resp);
            free(resp);
        } else {
            printf("Case %d: <no response>\n", i + 1);
        }
    }

    stop_server(&server);
    return 0;
}