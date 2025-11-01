#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

typedef struct {
    const char* code;
    double price;
} Product;

static const Product PRODUCTS[] = {
    {"P100", 10.00},
    {"P200", 25.50},
    {"P300", 99.99},
    {"P400", 5.25},
    {"P500", 1.50}
};
static const size_t PRODUCT_COUNT = sizeof(PRODUCTS)/sizeof(PRODUCTS[0]);

static bool is_valid_code(const char* code) {
    if (code == NULL) return false;
    size_t len = strlen(code);
    if (len < 1 || len > 20) return false;
    for (size_t i = 0; i < len; i++) {
        char c = code[i];
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9'))) {
            return false;
        }
    }
    return true;
}

static bool parse_quantity(const char* s, int* out) {
    if (s == NULL || out == NULL) return false;
    size_t len = strlen(s);
    if (len < 1 || len > 9) return false;
    for (size_t i = 0; i < len; i++) {
        if (s[i] < '0' || s[i] > '9') return false;
    }
    long v = strtol(s, NULL, 10);
    if (v < 1 || v > 100000) return false;
    *out = (int)v;
    return true;
}

// Function: return price for a product code, or negative if invalid/unknown
static double get_price_for_code(const char* code) {
    if (!is_valid_code(code)) return -1.0;
    for (size_t i = 0; i < PRODUCT_COUNT; i++) {
        if (strcmp(PRODUCTS[i].code, code) == 0) {
            return PRODUCTS[i].price;
        }
    }
    return -1.0;
}

// Function: calculate total, or negative on invalid input
static double calculate_total(const char* code, int qty) {
    if (!is_valid_code(code)) return -1.0;
    if (qty < 1 || qty > 100000) return -1.0;
    for (size_t i = 0; i < PRODUCT_COUNT; i++) {
        if (strcmp(PRODUCTS[i].code, code) == 0) {
            return PRODUCTS[i].price * (double)qty;
        }
    }
    return -1.0;
}

static void send_all(int fd, const char* data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, data + sent, len - sent, 0);
        if (n <= 0) break;
        sent += (size_t)n;
    }
}

static void send_response(int client_fd, int status, const char* json_fmt, ...) {
    const char* status_text = "OK";
    if (status == 400) status_text = "Bad Request";
    else if (status == 404) status_text = "Not Found";
    else if (status == 405) status_text = "Method Not Allowed";
    else if (status == 500) status_text = "Internal Server Error";

    char body[512];
    va_list ap;
    va_start(ap, json_fmt);
    int body_len = vsnprintf(body, sizeof(body), json_fmt, ap);
    va_end(ap);
    if (body_len < 0) body_len = 0;
    if (body_len >= (int)sizeof(body)) body_len = (int)sizeof(body) - 1;

    char header[256];
    int hdr_len = snprintf(header, sizeof(header),
                           "HTTP/1.1 %d %s\r\n"
                           "Content-Type: application/json; charset=utf-8\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n\r\n",
                           status, status_text, body_len);
    if (hdr_len < 0) hdr_len = 0;
    if (hdr_len > (int)sizeof(header)) hdr_len = (int)sizeof(header);

    send_all(client_fd, header, (size_t)hdr_len);
    send_all(client_fd, body, (size_t)body_len);
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static void url_decode(const char* in, char* out, size_t outsz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0' && oi + 1 < outsz; i++) {
        if (in[i] == '%' && in[i+1] && in[i+2]) {
            int h = hexval(in[i+1]);
            int l = hexval(in[i+2]);
            if (h >= 0 && l >= 0) {
                out[oi++] = (char)((h << 4) | l);
                i += 2;
            } else {
                out[oi++] = in[i];
            }
        } else if (in[i] == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = in[i];
        }
    }
    out[oi] = '\0';
}

static void handle_client(int client_fd) {
    char req[4096];
    memset(req, 0, sizeof(req));
    ssize_t r = recv(client_fd, req, sizeof(req)-1, 0);
    if (r <= 0) {
        close(client_fd);
        return;
    }
    if (strncmp(req, "GET ", 4) != 0) {
        send_response(client_fd, 405, "{\"error\":\"method not allowed\"}");
        close(client_fd);
        return;
    }
    // Extract path
    char* path_start = req + 4;
    char* space = strchr(path_start, ' ');
    if (!space) {
        send_response(client_fd, 400, "{\"error\":\"invalid input\"}");
        close(client_fd);
        return;
    }
    size_t path_len = (size_t)(space - path_start);
    if (path_len >= 1024) path_len = 1023;
    char fullpath[1024];
    memcpy(fullpath, path_start, path_len);
    fullpath[path_len] = '\0';

    char path[1024] = {0};
    char query[1024] = {0};
    char* qmark = strchr(fullpath, '?');
    if (qmark) {
        size_t p_len = (size_t)(qmark - fullpath);
        if (p_len >= sizeof(path)) p_len = sizeof(path)-1;
        memcpy(path, fullpath, p_len);
        path[p_len] = '\0';
        size_t q_len = strlen(qmark+1);
        if (q_len >= sizeof(query)) q_len = sizeof(query)-1;
        memcpy(query, qmark+1, q_len);
        query[q_len] = '\0';
    } else {
        size_t p_len = strlen(fullpath);
        if (p_len >= sizeof(path)) p_len = sizeof(path)-1;
        memcpy(path, fullpath, p_len);
        path[p_len] = '\0';
    }

    if (strcmp(path, "/price") != 0) {
        send_response(client_fd, 404, "{\"error\":\"not found\"}");
        close(client_fd);
        return;
    }

    // Parse query params
    char code_raw[64] = {0};
    char qty_raw[32] = {0};
    size_t i = 0;
    while (query[i] != '\0') {
        size_t start = i;
        while (query[i] != '&' && query[i] != '\0') i++;
        size_t seg_len = i - start;
        if (seg_len > 0 && seg_len < 256) {
            char seg[256];
            memcpy(seg, &query[start], seg_len);
            seg[seg_len] = '\0';
            char* eq = strchr(seg, '=');
            if (eq) {
                *eq = '\0';
                const char* key = seg;
                const char* val = eq + 1;
                if (strcmp(key, "code") == 0) {
                    url_decode(val, code_raw, sizeof(code_raw));
                } else if (strcmp(key, "qty") == 0) {
                    url_decode(val, qty_raw, sizeof(qty_raw));
                }
            }
        }
        if (query[i] == '&') i++;
    }

    if (!is_valid_code(code_raw)) {
        send_response(client_fd, 400, "{\"error\":\"invalid input\"}");
        close(client_fd);
        return;
    }
    int qty = 0;
    if (!parse_quantity(qty_raw, &qty)) {
        send_response(client_fd, 400, "{\"error\":\"invalid input\"}");
        close(client_fd);
        return;
    }

    double unit = get_price_for_code(code_raw);
    if (unit < 0.0) {
        send_response(client_fd, 404, "{\"error\":\"not found\"}");
        close(client_fd);
        return;
    }
    double total = unit * (double)qty;

    char body[256];
    int n = snprintf(body, sizeof(body),
                     "{\"code\":\"%s\",\"unitPrice\":%.2f,\"quantity\":%d,\"total\":%.2f}",
                     code_raw, unit, qty, total);
    if (n < 0 || n >= (int)sizeof(body)) {
        send_response(client_fd, 500, "{\"error\":\"server error\"}");
        close(client_fd);
        return;
    }
    send_response(client_fd, 200, "%s", body);
    close(client_fd);
}

int main(void) {
    // 5 tests
    printf("Test1 total P100 x 3 = %.2f\n", calculate_total("P100", 3)); // 30.00
    printf("Test2 total P200 x 2 = %.2f\n", calculate_total("P200", 2)); // 51.00
    printf("Test3 price P300 = %.2f\n", get_price_for_code("P300"));     // 99.99
    printf("Test4 invalid code -> %.2f\n", get_price_for_code("BAD!"));  // -1.0
    printf("Test5 invalid qty -> %.2f\n", calculate_total("P100", 0));   // -1.0

    signal(SIGPIPE, SIG_IGN);
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) {
        perror("socket");
        return 1;
    }
    int opt = 1;
    if (setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(srv);
        return 1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(5000);

    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(srv);
        return 1;
    }
    if (listen(srv, 16) < 0) {
        perror("listen");
        close(srv);
        return 1;
    }
    printf("Server running on http://0.0.0.0:5000\n");

    for (;;) {
        struct sockaddr_in cli;
        socklen_t clilen = sizeof(cli);
        int cfd = accept(srv, (struct sockaddr*)&cli, &clilen);
        if (cfd < 0) {
            continue;
        }
        handle_client(cfd);
    }

    close(srv);
    return 0;
}