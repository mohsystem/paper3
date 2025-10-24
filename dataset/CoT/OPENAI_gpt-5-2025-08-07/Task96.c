#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*
Chain-of-Through process:
1. Understand: TCP echo server on specified port; echo back messages.
2. Security: Bind to localhost; timeouts; limit input; close sockets; handle errors.
3. Implementation: Server thread accept loop; per-connection handling with bounded reads.
4. Review: Verify cleanup and error handling.
5. Output: Final code.
*/

#define MAX_LINE 8192
#define BACKLOG 50

typedef struct {
    int listen_fd;
    int port;
    volatile bool running;
    pthread_t thread;
} EchoServerC;

static bool read_line_limited(int fd, char *buf, size_t buflen) {
    size_t pos = 0;
    while (1) {
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n == 0) {
            return pos > 0;
        } else if (n < 0) {
            if (errno == EINTR) continue;
            return false;
        }
        if (ch == '\n') break;
        if (ch != '\r') {
            if (pos >= buflen - 1) {
                errno = EMSGSIZE;
                return false;
            }
            buf[pos++] = ch;
        }
    }
    buf[pos] = '\0';
    return true;
}

static void *client_handler(void *arg) {
    int fd = (int)(intptr_t)arg;

    struct timeval tv;
    tv.tv_sec = 30;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    char line[MAX_LINE + 1];
    while (1) {
        bool ok = read_line_limited(fd, line, sizeof(line));
        if (!ok) break;
        size_t len = strlen(line);
        // send line + '\n'
        if (send(fd, line, len, 0) < 0) break;
        if (send(fd, "\n", 1, 0) < 0) break;
    }
    close(fd);
    return NULL;
}

static void *accept_loop(void *arg) {
    EchoServerC *srv = (EchoServerC *)arg;
    while (srv->running) {
        struct sockaddr_in cli;
        socklen_t clilen = sizeof(cli);
        int cfd = accept(srv->listen_fd, (struct sockaddr *)&cli, &clilen);
        if (cfd < 0) {
            if (!srv->running) break;
            if (errno == EINTR) continue;
            usleep(10000);
            continue;
        }
        pthread_t t;
        if (pthread_create(&t, NULL, client_handler, (void *)(intptr_t)cfd) == 0) {
            pthread_detach(t);
        } else {
            close(cfd);
        }
    }
    return NULL;
}

static int start_echo_server(int port, EchoServerC *out) {
    memset(out, 0, sizeof(*out));
    out->listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (out->listen_fd < 0) return -1;

    int opt = 1;
    setsockopt(out->listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(port);

    if (bind(out->listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(out->listen_fd);
        return -1;
    }
    if (listen(out->listen_fd, BACKLOG) < 0) {
        close(out->listen_fd);
        return -1;
    }

    socklen_t len = sizeof(addr);
    if (getsockname(out->listen_fd, (struct sockaddr *)&addr, &len) == 0) {
        out->port = ntohs(addr.sin_port);
    } else {
        close(out->listen_fd);
        return -1;
    }

    out->running = true;
    if (pthread_create(&out->thread, NULL, accept_loop, out) != 0) {
        close(out->listen_fd);
        out->running = false;
        return -1;
    }
    return out->port;
}

static void stop_echo_server(EchoServerC *srv) {
    if (!srv->running) return;
    srv->running = false;
    shutdown(srv->listen_fd, SHUT_RDWR);
    close(srv->listen_fd);
    pthread_join(srv->thread, NULL);
}

static int echo_client(const char *host, int port, const char *message, char *out, size_t outsz) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;

    struct timeval tv;
    tv.tv_sec = 5; tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    size_t len = strlen(message);
    if (send(fd, message, len, 0) < 0) {
        close(fd);
        return -1;
    }
    if (send(fd, "\n", 1, 0) < 0) {
        close(fd);
        return -1;
    }

    // Read until newline
    size_t pos = 0;
    while (1) {
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n == 0) break;
        if (n < 0) {
            if (errno == EINTR) continue;
            close(fd);
            return -1;
        }
        if (ch == '\n') break;
        if (ch != '\r') {
            if (pos >= outsz - 1) {
                close(fd);
                return -1;
            }
            out[pos++] = ch;
        }
    }
    out[pos] = '\0';
    close(fd);
    return 0;
}

int main(void) {
    EchoServerC srv;
    int port = start_echo_server(0, &srv);
    if (port <= 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    const char *host = "127.0.0.1";
    const char *tests[5] = {
        "Hello",
        "",
        "The quick brown fox jumps over the lazy dog",
        "1234567890!@#$%^&*()_+",
        "ASCII only in C test"
    };

    for (int i = 0; i < 5; ++i) {
        char resp[MAX_LINE + 1];
        if (echo_client(host, port, tests[i], resp, sizeof(resp)) == 0) {
            printf("Test %d -> %s\n", i + 1, resp);
        } else {
            printf("Test %d -> error\n", i + 1);
        }
    }

    stop_echo_server(&srv);
    return 0;
}