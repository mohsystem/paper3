#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MESSAGE_BYTES 4096
#define BACKLOG 50
#define SOCKET_TIMEOUT_SEC 5

typedef struct {
    int listen_fd;
    volatile sig_atomic_t running;
    pthread_t thread;
    uint16_t port;
} ServerHandle;

static void close_fd_safe(int fd) {
    if (fd >= 0) close(fd);
}

static int set_socket_timeouts(int fd) {
    struct timeval tv;
    tv.tv_sec = SOCKET_TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0) return -1;
    if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) return -1;
    return 0;
}

static int read_line_limited(int fd, char* buf, size_t buf_size, size_t* out_len) {
    if (!buf || buf_size == 0) return -1;
    size_t count = 0;
    while (count < MAX_MESSAGE_BYTES && count + 1 < buf_size) {
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n == 0) {
            if (count == 0) return 1; // EOF, no data
            break;
        } else if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        if (ch == '\n') break;
        if (ch == '\r') continue;
        buf[count++] = ch;
    }
    buf[count] = '\0';
    if (out_len) *out_len = count;
    return 0;
}

static void* client_thread(void* arg) {
    int fd = *(int*)arg;
    free(arg);
    if (set_socket_timeouts(fd) != 0) {
        close_fd_safe(fd);
        return NULL;
    }
    char buf[MAX_MESSAGE_BYTES + 1];
    size_t len = 0;
    while (1) {
        int r = read_line_limited(fd, buf, sizeof(buf), &len);
        if (r == 1) break;         // EOF
        if (r < 0) break;          // error
        // Echo back with newline
        size_t total = len + 1;
        char* out = (char*)malloc(total);
        if (!out) break;
        memcpy(out, buf, len);
        out[len] = '\n';
        size_t sent = 0;
        while (sent < total) {
            ssize_t n = send(fd, out + sent, total - sent, 0);
            if (n < 0) {
                if (errno == EINTR) continue;
                break;
            }
            sent += (size_t)n;
        }
        free(out);
        if (sent < total) break;
    }
    close_fd_safe(fd);
    return NULL;
}

static void* accept_loop(void* arg) {
    ServerHandle* h = (ServerHandle*)arg;
    while (h->running) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int cfd = accept(h->listen_fd, (struct sockaddr*)&caddr, &clen);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            if (!h->running) break;
            continue;
        }
        int* pfd = (int*)malloc(sizeof(int));
        if (!pfd) {
            close_fd_safe(cfd);
            continue;
        }
        *pfd = cfd;
        pthread_t t;
        if (pthread_create(&t, NULL, client_thread, pfd) == 0) {
            pthread_detach(t);
        } else {
            free(pfd);
            close_fd_safe(cfd);
        }
    }
    return NULL;
}

ServerHandle* start_echo_server(uint16_t port) {
    if (port > 65535) return NULL;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return NULL;
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // 127.0.0.1
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close_fd_safe(fd);
        return NULL;
    }
    if (listen(fd, BACKLOG) != 0) {
        close_fd_safe(fd);
        return NULL;
    }
    socklen_t len = sizeof(addr);
    if (getsockname(fd, (struct sockaddr*)&addr, &len) != 0) {
        close_fd_safe(fd);
        return NULL;
    }
    uint16_t assigned_port = ntohs(addr.sin_port);

    ServerHandle* h = (ServerHandle*)malloc(sizeof(ServerHandle));
    if (!h) {
        close_fd_safe(fd);
        return NULL;
    }
    h->listen_fd = fd;
    h->running = 1;
    h->port = assigned_port;
    if (pthread_create(&h->thread, NULL, accept_loop, h) != 0) {
        close_fd_safe(fd);
        free(h);
        return NULL;
    }
    return h;
}

void stop_echo_server(ServerHandle* h) {
    if (!h) return;
    h->running = 0;
    shutdown(h->listen_fd, SHUT_RDWR);
    close_fd_safe(h->listen_fd);
    pthread_join(h->thread, NULL);
    free(h);
}

int send_echo(const char* host, uint16_t port, const char* message, char* out_buf, size_t out_buf_size) {
    if (!host || !out_buf || out_buf_size == 0) return -1;
    size_t msg_len = message ? strlen(message) : 0;
    if (msg_len > MAX_MESSAGE_BYTES) return -2;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -3;
    if (set_socket_timeouts(fd) != 0) {
        close_fd_safe(fd);
        return -3;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close_fd_safe(fd);
        return -3;
    }
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        close_fd_safe(fd);
        return -3;
    }

    // Send message + '\n'
    size_t total = msg_len + 1;
    char* tosend = (char*)malloc(total);
    if (!tosend) {
        close_fd_safe(fd);
        return -3;
    }
    if (message && msg_len > 0) memcpy(tosend, message, msg_len);
    tosend[msg_len] = '\n';

    size_t sent = 0;
    while (sent < total) {
        ssize_t n = send(fd, tosend + sent, total - sent, 0);
        if (n < 0) {
            if (errno == EINTR) continue;
            free(tosend);
            close_fd_safe(fd);
            return -3;
        }
        sent += (size_t)n;
    }
    free(tosend);

    // Read reply line
    size_t count = 0;
    while (count + 1 < out_buf_size && count < MAX_MESSAGE_BYTES) {
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n == 0) break;
        if (n < 0) {
            if (errno == EINTR) continue;
            close_fd_safe(fd);
            return -3;
        }
        if (ch == '\n') break;
        if (ch == '\r') continue;
        out_buf[count++] = ch;
    }
    out_buf[count] = '\0';
    close_fd_safe(fd);
    return (int)count;
}

int main(void) {
    ServerHandle* h = start_echo_server(0);
    if (!h) {
        printf("Failed to start server\n");
        return 1;
    }
    uint16_t port = h->port;
    const char* tests[5] = {
        "Hello, world!",
        "",
        "The quick brown fox jumps over 13 lazy dogs.",
        "Symbols: !@#$%^&*()_+-=[]{}|;:',.<>/?",
        "One more test line"
    };
    char buf[MAX_MESSAGE_BYTES + 1];
    for (int i = 0; i < 5; ++i) {
        int n = send_echo("127.0.0.1", port, tests[i], buf, sizeof(buf));
        if (n >= 0) {
            printf("Sent: [%s] | Echoed: [%s]\n", tests[i], buf);
        } else {
            printf("Echo failed for test %d\n", i + 1);
        }
    }
    stop_echo_server(h);
    return 0;
}