#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_PORT 8082

static volatile int g_stop = 0;

typedef struct {
    char username[33];
    char role[16];
} user_row;

static user_row g_db[128];
static size_t g_db_count = 0;

static int is_valid_username(const char *u) {
    if (u == NULL) return 0;
    size_t len = strlen(u);
    if (len < 1 || len > 32) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = u[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
    return -1;
}

static int url_decode(const char *in, char *out, size_t outsz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0'; i++) {
        if (oi + 1 >= outsz) return 0;
        if (in[i] == '%' && in[i + 1] && in[i + 2]) {
            int h1 = hexval(in[i + 1]);
            int h2 = hexval(in[i + 2]);
            if (h1 < 0 || h2 < 0) return 0;
            out[oi++] = (char)((h1 << 4) | h2);
            i += 2;
        } else if (in[i] == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = in[i];
        }
    }
    out[oi] = '\0';
    return 1;
}

static int execute_is_moderator_query(const char *username) {
    for (size_t i = 0; i < g_db_count; i++) {
        if (strcmp(g_db[i].username, username) == 0) {
            return strcmp(g_db[i].role, "moderator") == 0;
        }
    }
    return 0;
}

static void write_full(int fd, const char *data, size_t len) {
    size_t off = 0;
    while (off < len) {
        ssize_t n = send(fd, data + off, len - off, 0);
        if (n <= 0) break;
        off += (size_t)n;
    }
}

static void *server_thread(void *arg) {
    (void)arg;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return NULL;

    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(SERVER_PORT);
    if (bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sfd);
        return NULL;
    }
    if (listen(sfd, 16) < 0) {
        close(sfd);
        return NULL;
    }

    while (!g_stop) {
        struct sockaddr_in cli;
        socklen_t clilen = sizeof(cli);
        int cfd = accept(sfd, (struct sockaddr *)&cli, &clilen);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            if (g_stop) break;
            continue;
        }
        char buf[2048];
        memset(buf, 0, sizeof(buf));
        ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            close(cfd);
            continue;
        }
        // parse
        char method[8] = {0};
        char target[1024] = {0};
        if (sscanf(buf, "%7s %1023s", method, target) != 2) {
            close(cfd);
            continue;
        }
        if (strcmp(method, "GET") != 0) {
            const char *body = "{\"error\":\"method not allowed\"}";
            char header[256];
            int hl = snprintf(header, sizeof(header),
                              "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                              strlen(body));
            write_full(cfd, header, (size_t)hl);
            write_full(cfd, body, strlen(body));
            close(cfd);
            continue;
        }

        char path[1024] = {0};
        char query[1024] = {0};
        char *qmark = strchr(target, '?');
        if (qmark) {
            size_t plen = (size_t)(qmark - target);
            if (plen >= sizeof(path)) plen = sizeof(path) - 1;
            memcpy(path, target, plen);
            path[plen] = '\0';
            strncpy(query, qmark + 1, sizeof(query) - 1);
            query[sizeof(query) - 1] = '\0';
        } else {
            strncpy(path, target, sizeof(path) - 1);
            path[sizeof(path) - 1] = '\0';
            query[0] = '\0';
        }

        if (strcmp(path, "/is_moderator") != 0) {
            const char *body = "{\"error\":\"not found\"}";
            char header[256];
            int hl = snprintf(header, sizeof(header),
                              "HTTP/1.1 404 Not Found\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                              strlen(body));
            write_full(cfd, header, (size_t)hl);
            write_full(cfd, body, strlen(body));
            close(cfd);
            continue;
        }

        char username_enc[256] = {0};
        char username[256] = {0};
        // parse username param
        // Look for "username="
        const char *p = strstr(query, "username=");
        if (p) {
            p += 9;
            size_t i = 0;
            while (*p && *p != '&' && i + 1 < sizeof(username_enc)) {
                username_enc[i++] = *p++;
            }
            username_enc[i] = '\0';
        }
        if (username_enc[0] == '\0' || !url_decode(username_enc, username, sizeof(username))) {
            const char *body = "{\"error\":\"invalid username\"}";
            char header[256];
            int hl = snprintf(header, sizeof(header),
                              "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                              strlen(body));
            write_full(cfd, header, (size_t)hl);
            write_full(cfd, body, strlen(body));
            close(cfd);
            continue;
        }
        if (!is_valid_username(username)) {
            const char *body = "{\"error\":\"invalid username\"}";
            char header[256];
            int hl = snprintf(header, sizeof(header),
                              "HTTP/1.1 400 Bad Request\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
                              strlen(body));
            write_full(cfd, header, (size_t)hl);
            write_full(cfd, body, strlen(body));
            close(cfd);
            continue;
        }

        int is_mod = execute_is_moderator_query(username);
        char body[256];
        int bl = snprintf(body, sizeof(body), "{\"username\":\"%s\",\"is_moderator\":%s}", username, is_mod ? "true" : "false");
        if (bl < 0) bl = 0;
        char header[256];
        int hl = snprintf(header, sizeof(header),
                          "HTTP/1.1 200 OK\r\nContent-Type: application/json; charset=utf-8\r\nContent-Length: %d\r\nConnection: close\r\n\r\n", bl);
        write_full(cfd, header, (size_t)hl);
        write_full(cfd, body, (size_t)bl);
        close(cfd);
    }
    return NULL;
}

static void http_get_local(const char *username, char *out, size_t outsz) {
    if (!out || outsz == 0) return;
    out[0] = '\0';
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return;

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(SERVER_PORT);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return;
    }
    char path[256];
    snprintf(path, sizeof(path), "/is_moderator?username=%s", username);
    char req[512];
    int rl = snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: 127.0.0.1\r\nConnection: close\r\n\r\n", path);
    if (rl < 0) {
        close(fd);
        return;
    }
    size_t sent = 0;
    while (sent < (size_t)rl) {
        ssize_t n = send(fd, req + sent, (size_t)rl - sent, 0);
        if (n <= 0) break;
        sent += (size_t)n;
    }

    char resp[4096];
    size_t off = 0;
    ssize_t n;
    while ((n = recv(fd, resp + off, sizeof(resp) - off, 0)) > 0) {
        off += (size_t)n;
        if (off >= sizeof(resp)) break;
    }
    close(fd);
    // Extract body
    resp[off < sizeof(resp) ? off : sizeof(resp) - 1] = '\0';
    char *body = strstr(resp, "\r\n\r\n");
    if (body) {
        body += 4;
        strncpy(out, body, outsz - 1);
        out[outsz - 1] = '\0';
    } else {
        strncpy(out, resp, outsz - 1);
        out[outsz - 1] = '\0';
    }
}

static void db_seed(void) {
    g_db_count = 0;
    const char *entries[][2] = {
        {"alice", "moderator"},
        {"bob", "user"},
        {"carol", "moderator"},
        {"dave", "user"},
        {"eve", "moderator"}
    };
    for (size_t i = 0; i < sizeof(entries)/sizeof(entries[0]); i++) {
        strncpy(g_db[g_db_count].username, entries[i][0], sizeof(g_db[g_db_count].username) - 1);
        g_db[g_db_count].username[sizeof(g_db[g_db_count].username) - 1] = '\0';
        strncpy(g_db[g_db_count].role, entries[i][1], sizeof(g_db[g_db_count].role) - 1);
        g_db[g_db_count].role[sizeof(g_db[g_db_count].role) - 1] = '\0';
        g_db_count++;
    }
}

int main(void) {
    // "Connect to the database" (simulated in-memory)
    db_seed();

    pthread_t tid;
    if (pthread_create(&tid, NULL, server_thread, NULL) != 0) {
        fprintf(stderr, "Failed to start server\n");
        return 1;
    }

    const char *tests[5] = {"alice", "bob", "carol", "dave", "unknown"};
    for (int i = 0; i < 5; i++) {
        char out[1024];
        http_get_local(tests[i], out, sizeof(out));
        printf("%s\n", out);
    }

    g_stop = 1;
    // Nudge server
    char dummy[1024];
    http_get_local("alice", dummy, sizeof(dummy));
    pthread_join(tid, NULL);
    return 0;
}