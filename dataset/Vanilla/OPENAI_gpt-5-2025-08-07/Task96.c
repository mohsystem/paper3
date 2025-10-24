#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

static volatile int c_stop_flag = 0;
static int c_listen_fd = -1;
static pthread_t c_server_thread;

static void* c_server_loop(void* arg) {
    int lfd = (int)(intptr_t)arg;
    while (!c_stop_flag) {
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(lfd, &rfds);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 500000; // 0.5s
        int rv = select(lfd + 1, &rfds, NULL, NULL, &tv);
        if (rv < 0) {
            if (errno == EINTR) continue;
            break;
        }
        if (rv == 0) continue;
        if (FD_ISSET(lfd, &rfds)) {
            struct sockaddr_in cli;
            socklen_t clilen = sizeof(cli);
            int cfd = accept(lfd, (struct sockaddr*)&cli, &clilen);
            if (cfd < 0) {
                if (errno == EINTR) continue;
                if (errno == EBADF) break;
                continue;
            }
            char buf[4096];
            while (1) {
                ssize_t n = recv(cfd, buf, sizeof(buf), 0);
                if (n <= 0) break;
                ssize_t sent = 0;
                while (sent < n) {
                    ssize_t m = send(cfd, buf + sent, (size_t)(n - sent), 0);
                    if (m <= 0) { n = -1; break; }
                    sent += m;
                }
                if (n <= 0) break;
            }
            close(cfd);
        }
    }
    close(lfd);
    return NULL;
}

static unsigned short start_echo_server(unsigned short port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 0;
    }
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(fd);
        return 0;
    }
    if (listen(fd, 128) < 0) {
        perror("listen");
        close(fd);
        return 0;
    }
    struct sockaddr_in real;
    socklen_t len = sizeof(real);
    if (getsockname(fd, (struct sockaddr*)&real, &len) < 0) {
        perror("getsockname");
        close(fd);
        return 0;
    }
    unsigned short actual_port = ntohs(real.sin_port);
    c_listen_fd = fd;
    c_stop_flag = 0;
    if (pthread_create(&c_server_thread, NULL, c_server_loop, (void*)(intptr_t)fd) != 0) {
        perror("pthread_create");
        close(fd);
        c_listen_fd = -1;
        return 0;
    }
    return actual_port;
}

static void stop_echo_server() {
    c_stop_flag = 1;
    if (c_listen_fd >= 0) {
        // Wake the select by connecting
        int wfd = socket(AF_INET, SOCK_STREAM, 0);
        if (wfd >= 0) {
            struct sockaddr_in addr;
            socklen_t len = sizeof(addr);
            if (getsockname(c_listen_fd, (struct sockaddr*)&addr, &len) == 0) {
                connect(wfd, (struct sockaddr*)&addr, sizeof(addr));
            }
            close(wfd);
        }
    }
    pthread_join(c_server_thread, NULL);
    c_listen_fd = -1;
}

static ssize_t echo_once(const char* host, unsigned short port, const char* msg, char* out, size_t outcap) {
    if (!host || !msg || !out || outcap == 0) return -1;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return -1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &addr.sin_addr) != 1) {
        close(fd);
        return -1;
    }
    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }
    size_t len = strlen(msg);
    const char* p = msg;
    size_t left = len;
    while (left > 0) {
        ssize_t n = send(fd, p, left, 0);
        if (n <= 0) { close(fd); return -1; }
        p += n;
        left -= (size_t)n;
    }
    shutdown(fd, SHUT_WR);
    size_t total = 0;
    char buf[4096];
    while (1) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        size_t tocpy = (size_t)n;
        if (total + tocpy >= outcap) tocpy = (outcap - 1) - total;
        if (tocpy > 0) {
            memcpy(out + total, buf, tocpy);
            total += tocpy;
        }
        if (total >= outcap - 1) break;
    }
    out[total] = '\0';
    close(fd);
    return (ssize_t)total;
}

int main(void) {
    unsigned short port = start_echo_server(0);
    if (port == 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }
    const char* tests[5] = {
        "hello",
        "world",
        "こんにちは",
        "1234567890",
        "echo test with spaces"
    };
    char buf[8192];
    for (int i = 0; i < 5; ++i) {
        ssize_t n = echo_once("127.0.0.1", port, tests[i], buf, sizeof(buf));
        printf("Sent:   %s\n", tests[i]);
        printf("Echoed: %s\n", (n >= 0 ? buf : "(error)"));
        printf("Match:  %s\n", (n >= 0 && strcmp(tests[i], buf) == 0) ? "true" : "false");
        printf("---\n");
    }
    stop_echo_server();
    return 0;
}