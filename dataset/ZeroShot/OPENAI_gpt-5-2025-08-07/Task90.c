#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>

#define MAX_MSG_LEN 512
#define CLIENT_IDLE_TIMEOUT_SEC 60
#define DEFAULT_MAX_CLIENTS 100
#define MAX_FD 1024

typedef struct {
    int fd;
    time_t last_active;
    char buf[MAX_MSG_LEN + 1];
    size_t buf_len;
} client_t;

static int is_valid_port(int port) {
    return port >= 1024 && port <= 65535;
}

static size_t sanitize_message(const char* input, size_t max_len, char* out, size_t out_size) {
    if (!input || !out || out_size == 0) return 0;
    size_t count = 0;
    size_t out_pos = 0;
    for (size_t i = 0; input[i] != '\0' && count < max_len; ++i) {
        unsigned char c = (unsigned char)input[i];
        if (c == '\r' || c == '\n') {
            if (out_pos == 0 || out[out_pos - 1] != ' ') {
                if (out_pos + 1 >= out_size) break;
                out[out_pos++] = ' ';
                count++;
            }
        } else if (c == '\t' || (c >= 0x20 && !(c >= 0x7F && c <= 0x9F))) {
            if (out_pos + 1 >= out_size) break;
            out[out_pos++] = (char)c;
            count++;
        }
    }
    // Trim
    while (out_pos > 0 && (out[0] == ' ' || out[0] == '\t')) {
        memmove(out, out + 1, out_pos - 1);
        out_pos--;
    }
    while (out_pos > 0 && (out[out_pos - 1] == ' ' || out[out_pos - 1] == '\t')) {
        out_pos--;
    }
    if (out_pos >= out_size) out_pos = out_size - 1;
    out[out_pos] = '\0';
    return out_pos;
}

typedef struct {
    int listen_fd;
    int max_clients;
    int running;
    client_t clients[DEFAULT_MAX_CLIENTS];
} chat_server_t;

static void server_init(chat_server_t* srv, int max_clients) {
    srv->listen_fd = -1;
    srv->max_clients = (max_clients < 1) ? 1 : max_clients;
    srv->running = 0;
    for (int i = 0; i < DEFAULT_MAX_CLIENTS; ++i) {
        srv->clients[i].fd = -1;
        srv->clients[i].buf_len = 0;
        srv->clients[i].last_active = 0;
    }
}

static void close_client(client_t* c) {
    if (c->fd >= 0) {
        close(c->fd);
        c->fd = -1;
        c->buf_len = 0;
    }
}

static void server_stop(chat_server_t* srv) {
    srv->running = 0;
    if (srv->listen_fd >= 0) {
        close(srv->listen_fd);
        srv->listen_fd = -1;
    }
    for (int i = 0; i < srv->max_clients && i < DEFAULT_MAX_CLIENTS; ++i) {
        close_client(&srv->clients[i]);
    }
}

static int send_line(int fd, const char* line) {
    if (!line) return -1;
    size_t len = strnlen(line, MAX_MSG_LEN);
    char buf[MAX_MSG_LEN + 2];
    if (len > MAX_MSG_LEN) len = MAX_MSG_LEN;
    memcpy(buf, line, len);
    buf[len++] = '\n';
    ssize_t n = send(fd, buf, len, 0);
    return (n < 0) ? -1 : 0;
}

static void broadcast(chat_server_t* srv, const char* msg, int sender_fd) {
    for (int i = 0; i < srv->max_clients && i < DEFAULT_MAX_CLIENTS; ++i) {
        if (srv->clients[i].fd >= 0 && srv->clients[i].fd != sender_fd) {
            send_line(srv->clients[i].fd, msg);
        }
    }
}

static int add_client(chat_server_t* srv, int fd) {
    for (int i = 0; i < srv->max_clients && i < DEFAULT_MAX_CLIENTS; ++i) {
        if (srv->clients[i].fd < 0) {
            srv->clients[i].fd = fd;
            srv->clients[i].last_active = time(NULL);
            srv->clients[i].buf_len = 0;
            send_line(fd, "Welcome to Secure Chat Server");
            return 0;
        }
    }
    return -1;
}

static int server_start(chat_server_t* srv, int port) {
    if (!is_valid_port(port)) return 0;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 0;
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((uint16_t)port);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(fd);
        return 0;
    }
    if (listen(fd, (srv->max_clients < 50 ? srv->max_clients : 50)) < 0) {
        close(fd);
        return 0;
    }
    srv->listen_fd = fd;
    srv->running = 1;
    return 1;
}

static void server_run(chat_server_t* srv) {
    while (srv->running) {
        fd_set rfds;
        FD_ZERO(&rfds);
        int maxfd = srv->listen_fd;
        FD_SET(srv->listen_fd, &rfds);
        for (int i = 0; i < srv->max_clients && i < DEFAULT_MAX_CLIENTS; ++i) {
            if (srv->clients[i].fd >= 0) {
                FD_SET(srv->clients[i].fd, &rfds);
                if (srv->clients[i].fd > maxfd) maxfd = srv->clients[i].fd;
            }
        }
        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        int ret = select(maxfd + 1, &rfds, NULL, NULL, &tv);
        if (ret < 0) {
            if (errno == EINTR) continue;
            break;
        }
        time_t now = time(NULL);
        // handle new connections
        if (FD_ISSET(srv->listen_fd, &rfds)) {
            struct sockaddr_in cli;
            socklen_t cl = sizeof(cli);
            int cfd = accept(srv->listen_fd, (struct sockaddr*)&cli, &cl);
            if (cfd >= 0) {
                if (add_client(srv, cfd) != 0) {
                    close(cfd);
                }
            }
        }
        // handle client data
        for (int i = 0; i < srv->max_clients && i < DEFAULT_MAX_CLIENTS; ++i) {
            client_t* c = &srv->clients[i];
            if (c->fd >= 0 && FD_ISSET(c->fd, &rfds)) {
                char buf[1024];
                ssize_t n = recv(c->fd, buf, sizeof(buf), 0);
                if (n <= 0) {
                    close_client(c);
                    continue;
                }
                c->last_active = now;
                // append to buffer
                size_t to_copy = (n > (ssize_t)(sizeof(c->buf) - 1 - c->buf_len)) ? (sizeof(c->buf) - 1 - c->buf_len) : (size_t)n;
                if (to_copy > 0) {
                    memcpy(c->buf + c->buf_len, buf, to_copy);
                    c->buf_len += to_copy;
                    c->buf[c->buf_len] = '\0';
                }
                // process lines
                char* line_start = c->buf;
                char* nl;
                while ((nl = strchr(line_start, '\n')) != NULL) {
                    *nl = '\0';
                    char sanitized[MAX_MSG_LEN + 1];
                    sanitize_message(line_start, MAX_MSG_LEN, sanitized, sizeof(sanitized));
                    if (sanitized[0] != '\0') {
                        broadcast(srv, sanitized, c->fd);
                    }
                    line_start = nl + 1;
                }
                // move remaining to start
                size_t remaining = c->buf + c->buf_len - line_start;
                memmove(c->buf, line_start, remaining);
                c->buf_len = remaining;
                c->buf[c->buf_len] = '\0';
            }
        }
        // idle timeouts
        for (int i = 0; i < srv->max_clients && i < DEFAULT_MAX_CLIENTS; ++i) {
            client_t* c = &srv->clients[i];
            if (c->fd >= 0 && now - c->last_active > CLIENT_IDLE_TIMEOUT_SEC) {
                close_client(c);
            }
        }
    }
    server_stop(srv);
}

static void run_tests(void) {
    int passed = 0;
    char out[1024];
    sanitize_message("Hello\0World", MAX_MSG_LEN, out, sizeof(out));
    if (strcmp(out, "HelloWorld") == 0) passed++;

    char longstr[601];
    memset(longstr, 'a', 600);
    longstr[600] = '\0';
    sanitize_message(longstr, 128, out, sizeof(out));
    if (strlen(out) == 128) passed++;

    if (is_valid_port(8080)) passed++;
    if (!is_valid_port(70000)) passed++;
    if (!is_valid_port(1023)) passed++;

    printf("Tests passed: %d/5\n", passed);
}

int main(int argc, char** argv) {
    run_tests();
    if (argc >= 2) {
        int port = atoi(argv[1]);
        chat_server_t srv;
        server_init(&srv, DEFAULT_MAX_CLIENTS);
        if (server_start(&srv, port)) {
            printf("Secure Chat Server running on port %d. Press Ctrl+C to stop.\n", port);
            server_run(&srv);
        } else {
            printf("Failed to start server. Invalid port or system error.\n");
        }
    }
    return 0;
}