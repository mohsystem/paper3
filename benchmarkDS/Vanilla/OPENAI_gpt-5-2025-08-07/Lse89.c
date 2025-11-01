// Simple POSIX-only HTTP server and client in one C file.
// Build: gcc -pthread file.c -o app
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

double price_for_code(const char* code) {
    if (!code) return 0.0;
    if (strcmp(code, "A100") == 0) return 10.0;
    if (strcmp(code, "B200") == 0) return 20.0;
    if (strcmp(code, "C300") == 0) return 7.5;
    if (strcmp(code, "D400") == 0) return 15.25;
    if (strcmp(code, "E500") == 0) return 5.75;
    return 0.0;
}

double total_price(const char* code, int qty) {
    if (qty < 0) qty = 0;
    return price_for_code(code) * qty;
}

static char from_hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return 0;
}

static char* url_decode(const char* s) {
    if (!s) return strdup("");
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '%' && i + 2 < len) {
            char v = (from_hex(s[i+1]) << 4) | from_hex(s[i+2]);
            out[oi++] = v;
            i += 2;
        } else if (s[i] == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = s[i];
        }
    }
    out[oi] = '\0';
    return out;
}

typedef struct {
    int running;
    int port;
} server_state_t;

static void parse_query(const char* q, char** code_out, int* qty_out) {
    *code_out = strdup("");
    *qty_out = 0;
    if (!q) return;
    char* qs = strdup(q);
    char* saveptr = NULL;
    for (char* tok = strtok_r(qs, "&", &saveptr); tok; tok = strtok_r(NULL, "&", &saveptr)) {
        char* eq = strchr(tok, '=');
        if (eq) {
            *eq = '\0';
            char* k = url_decode(tok);
            char* v = url_decode(eq + 1);
            if (strcmp(k, "code") == 0) {
                free(*code_out);
                *code_out = strdup(v);
            } else if (strcmp(k, "qty") == 0) {
                *qty_out = atoi(v);
            }
            free(k);
            free(v);
        } else {
            char* k = url_decode(tok);
            if (strcmp(k, "code") == 0) {
                free(*code_out);
                *code_out = strdup("");
            }
            free(k);
        }
    }
    free(qs);
}

static void* client_handler(void* arg) {
    int cfd = *(int*)arg;
    free(arg);
    char buf[4096];
    ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
    if (n <= 0) { close(cfd); return NULL; }
    buf[n] = '\0';

    char method[16] = {0}, path[2048] = {0}, version[16] = {0};
    sscanf(buf, "%15s %2047s %15s", method, path, version);

    int status = 200;
    char* body = NULL;
    const char* ct = "application/json; charset=utf-8";

    if (strcmp(method, "GET") != 0) {
        status = 405;
        body = strdup("{\"error\":\"method not allowed\"}");
    } else {
        char* route = strdup(path);
        char* query = NULL;
        char* qmark = strchr(route, '?');
        if (qmark) {
            *qmark = '\0';
            query = qmark + 1;
        }
        if (strcmp(route, "/price") != 0) {
            status = 404;
            body = strdup("{\"error\":\"not found\"}");
        } else {
            char* code = NULL;
            int qty = 0;
            parse_query(query, &code, &qty);
            double unit = price_for_code(code);
            double total = (qty < 0 ? 0 : qty) * unit;
            char tmp[256];
            snprintf(tmp, sizeof(tmp), "{\"code\":\"%s\",\"unitPrice\":%.2f,\"qty\":%d,\"total\":%.2f}", code, unit, qty, total);
            body = strdup(tmp);
            free(code);
        }
        free(route);
    }

    char header[512];
    snprintf(header, sizeof(header),
             "HTTP/1.1 %d OK\r\nContent-Type: %s\r\nContent-Length: %zu\r\nConnection: close\r\n\r\n",
             status, ct, strlen(body));
    send(cfd, header, strlen(header), 0);
    send(cfd, body, strlen(body), 0);
    free(body);
    close(cfd);
    return NULL;
}

static void* server_thread(void* arg) {
    server_state_t* st = (server_state_t*)arg;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return NULL;
    int opt = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(st->port);

    if (bind(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sfd);
        return NULL;
    }
    if (listen(sfd, 16) < 0) {
        close(sfd);
        return NULL;
    }

    while (st->running) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int* cfd = (int*)malloc(sizeof(int));
        *cfd = accept(sfd, (struct sockaddr*)&caddr, &clen);
        if (*cfd < 0) { free(cfd); continue; }
        pthread_t t;
        pthread_create(&t, NULL, client_handler, cfd);
        pthread_detach(t);
    }

    close(sfd);
    return NULL;
}

static char* http_get(const char* host, int port, const char* path) {
    int sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sfd < 0) return strdup("");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    if (connect(sfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sfd);
        return strdup("");
    }

    char req[1024];
    snprintf(req, sizeof(req), "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", path, host);
    send(sfd, req, strlen(req), 0);

    size_t cap = 4096, len = 0;
    char* out = (char*)malloc(cap);
    for (;;) {
        char buf[2048];
        ssize_t n = recv(sfd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        if (len + (size_t)n >= cap) {
            cap *= 2;
            out = (char*)realloc(out, cap);
        }
        memcpy(out + len, buf, (size_t)n);
        len += (size_t)n;
    }
    close(sfd);
    out[len] = '\0';
    return out;
}

int main(void) {
    const int port = 5000;
    server_state_t st;
    st.running = 1;
    st.port = port;

    pthread_t srv;
    pthread_create(&srv, NULL, server_thread, &st);

    usleep(200 * 1000);

    const char* tests[5] = {
        "/price?code=A100&qty=3",
        "/price?code=B200&qty=2",
        "/price?code=C300&qty=5",
        "/price?code=D400&qty=1",
        "/price?code=Z999&qty=4"
    };

    for (int i = 0; i < 5; ++i) {
        char* resp = http_get("127.0.0.1", port, tests[i]);
        char* body = strstr(resp, "\r\n\r\n");
        if (body) body += 4; else body = resp;
        printf("GET %s -> %s\n", tests[i], body);
        free(resp);
    }

    // Function tests
    printf("price_for_code('A100') = %.2f\n", price_for_code("A100"));
    printf("total_price('B200', 3) = %.2f\n", total_price("B200", 3));

    st.running = 0;
    // Unblock accept
    char* dummy = http_get("127.0.0.1", port, "/");
    free(dummy);
    pthread_join(srv, NULL);
    return 0;
}