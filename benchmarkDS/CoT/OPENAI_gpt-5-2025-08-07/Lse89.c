/* Chain-of-Through:
 * 1) Problem: Web app on port 5000 with function returning unit price and route /price?code=..&qty=.. to compute total.
 * 2) Security: Validate inputs, avoid overflow, safe parsing, restricted characters.
 * 3) Secure coding: use bounded buffers, check return values, minimal JSON output.
 * 4) Review: ensure no buffer overruns, safe socket operations, simple URL decode.
 * 5) Output: final secure code.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { const char* code; double price; } price_entry;

static const price_entry PRICES[] = {
    {"A100", 9.99},
    {"B200", 14.50},
    {"C300", 4.25},
    {"D400", 100.00},
    {"E500", 0.99},
};
static const size_t PRICES_LEN = sizeof(PRICES)/sizeof(PRICES[0]);

int is_valid_code(const char* code) {
    if (!code) return 0;
    size_t len = strlen(code);
    if (len == 0 || len > 20) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)code[i];
        if (!(isalnum(c) || c == '_' || c == '-')) return 0;
    }
    return 1;
}

int get_price(const char* code, double* out_price) {
    if (!is_valid_code(code) || !out_price) return 0;
    for (size_t i = 0; i < PRICES_LEN; ++i) {
        if (strcmp(PRICES[i].code, code) == 0) {
            *out_price = PRICES[i].price;
            return 1;
        }
    }
    return 0;
}

static int parse_int_safely(const char* s) {
    if (!s || !*s) return -1;
    size_t len = strlen(s);
    if (len > 10) return -1;
    long long val = 0;
    for (size_t i = 0; i < len; ++i) {
        if (!isdigit((unsigned char)s[i])) return -1;
        val = val * 10 + (s[i] - '0');
        if (val > 1000000LL) return -1;
    }
    if (val < 1) return -1;
    return (int)val;
}

static char hex_to_byte(char h) {
    if (h >= '0' && h <= '9') return (char)(h - '0');
    if (h >= 'a' && h <= 'f') return (char)(10 + (h - 'a'));
    if (h >= 'A' && h <= 'F') return (char)(10 + (h - 'A'));
    return 0;
}

static void urldecode(const char* in, char* out, size_t outsz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0' && oi + 1 < outsz; ++i) {
        if (in[i] == '%' && isxdigit((unsigned char)in[i+1]) && isxdigit((unsigned char)in[i+2])) {
            char hi = hex_to_byte(in[i+1]);
            char lo = hex_to_byte(in[i+2]);
            out[oi++] = (char)((hi << 4) | lo);
            i += 2;
        } else if (in[i] == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = in[i];
        }
    }
    if (oi < outsz) out[oi] = '\0';
}

static void send_response(int client, int status, const char* body) {
    char header[512];
    int blen = (int)strlen(body);
    const char* status_text = (status == 200) ? "OK" : (status == 400) ? "Bad Request" : (status == 404) ? "Not Found" : "Error";
    int hlen = snprintf(header, sizeof(header),
                        "HTTP/1.1 %d %s\r\n"
                        "Content-Type: application/json; charset=utf-8\r\n"
                        "X-Content-Type-Options: nosniff\r\n"
                        "Cache-Control: no-store\r\n"
                        "Connection: close\r\n"
                        "Content-Length: %d\r\n\r\n",
                        status, status_text, blen);
    if (hlen < 0) return;
    send(client, header, (size_t)hlen, 0);
    send(client, body, (size_t)blen, 0);
}

static void handle_client(int client) {
    char buf[2048];
    ssize_t n = recv(client, buf, sizeof(buf)-1, 0);
    if (n <= 0) { close(client); return; }
    buf[n] = '\0';
    char method[8] = {0};
    char target[1024] = {0};
    // Parse request line safely
    if (sscanf(buf, "%7s %1023s", method, target) != 2) {
        send_response(client, 400, "{\"error\":\"bad_request\"}");
        close(client);
        return;
    }
    if (strcmp(method, "GET") != 0) {
        send_response(client, 405, "{\"error\":\"method_not_allowed\"}");
        close(client);
        return;
    }
    // Separate path and query
    char path[1024] = {0};
    char query[1024] = {0};
    const char* qmark = strchr(target, '?');
    if (qmark) {
        size_t plen = (size_t)(qmark - target);
        if (plen >= sizeof(path)) plen = sizeof(path)-1;
        memcpy(path, target, plen);
        path[plen] = '\0';
        strncpy(query, qmark + 1, sizeof(query)-1);
    } else {
        strncpy(path, target, sizeof(path)-1);
    }
    if (strncmp(path, "/price", 6) != 0 || (path[6] != '\0' && path[6] != '?')) {
        send_response(client, 404, "{\"error\":\"not_found\"}");
        close(client);
        return;
    }
    // Parse params
    char code_enc[128] = {0};
    char qtys[64] = {0};
    char* saveptr = NULL;
    char qcopy[1024]; memset(qcopy, 0, sizeof(qcopy));
    strncpy(qcopy, query, sizeof(qcopy)-1);
    char* token = strtok_r(qcopy, "&", &saveptr);
    while (token) {
        char* eq = strchr(token, '=');
        if (eq) {
            *eq = '\0';
            const char* key = token;
            const char* val = eq + 1;
            if (strcmp(key, "code") == 0) {
                strncpy(code_enc, val, sizeof(code_enc)-1);
            } else if (strcmp(key, "qty") == 0) {
                strncpy(qtys, val, sizeof(qtys)-1);
            }
        }
        token = strtok_r(NULL, "&", &saveptr);
    }
    if (code_enc[0] == '\0' || qtys[0] == '\0') {
        send_response(client, 400, "{\"error\":\"missing_parameters\"}");
        close(client);
        return;
    }
    char code[128] = {0};
    urldecode(code_enc, code, sizeof(code));
    if (!is_valid_code(code)) {
        send_response(client, 400, "{\"error\":\"invalid_code\"}");
        close(client);
        return;
    }
    int qty = parse_int_safely(qtys);
    if (qty <= 0) {
        send_response(client, 400, "{\"error\":\"invalid_quantity\"}");
        close(client);
        return;
    }
    double unit = 0.0;
    if (!get_price(code, &unit)) {
        send_response(client, 404, "{\"error\":\"unknown_code\"}");
        close(client);
        return;
    }
    double total = unit * (double)qty;
    char body[256];
    // Format with 2 decimals
    snprintf(body, sizeof(body), "{\"code\":\"%s\",\"quantity\":%d,\"unit_price\":%.2f,\"total\":%.2f}", code, qty, unit, total);
    send_response(client, 200, body);
    close(client);
}

void start_server(int port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) { perror("socket"); return; }
    int opt = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(s); return; }
    if (listen(s, 16) < 0) { perror("listen"); close(s); return; }
    printf("C server started on port %d\n", port);
    while (1) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int c = accept(s, (struct sockaddr*)&caddr, &clen);
        if (c < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }
        handle_client(c);
    }
    close(s);
}

int main() {
    // 5 test cases (function)
    double p = 0.0;
    if (get_price("A100", &p)) printf("Test1 A100 x2 => %.2f\n", p * 2);
    if (get_price("B200", &p)) printf("Test2 B200 x1 => %.2f\n", p * 1);
    if (get_price("C300", &p)) printf("Test3 C300 x5 => %.2f\n", p * 5);
    if (get_price("D400", &p)) printf("Test4 D400 x10 => %.2f\n", p * 10);
    if (!get_price("Z999", &p)) printf("Test5 Z999 x3 => unknown\n");

    start_server(5000);
    return 0;
}