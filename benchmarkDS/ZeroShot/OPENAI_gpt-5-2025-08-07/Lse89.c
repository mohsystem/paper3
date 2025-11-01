#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef long long ll;

typedef struct {
    const char* code;
    ll cents;
} price_entry;

static price_entry PRICE_MAP[] = {
    {"A100", 1250},
    {"B200", 799},
    {"C300", 19900},
    {"D400", 325},
    {"E500", 99}
};
static const size_t PRICE_MAP_LEN = sizeof(PRICE_MAP)/sizeof(PRICE_MAP[0]);

static int is_valid_code(const char* s) {
    size_t len = 0;
    if (!s) return 0;
    for (; s[len] != '\0'; ++len) {
        if (!isalnum((unsigned char)s[len])) return 0;
        if (len > 16) return 0;
    }
    if (len < 2 || len > 16) return 0;
    return 1;
}

static void to_upper_inplace(char* s) {
    if (!s) return;
    for (; *s; ++s) *s = (char)toupper((unsigned char)*s);
}

// Returns unit price in cents, or -1 if not found/invalid
ll get_price(const char* code) {
    if (!code) return -1;
    char buf[32];
    snprintf(buf, sizeof(buf), "%s", code);
    to_upper_inplace(buf);
    if (!is_valid_code(buf)) return -1;
    for (size_t i = 0; i < PRICE_MAP_LEN; ++i) {
        if (strcmp(PRICE_MAP[i].code, buf) == 0) {
            return PRICE_MAP[i].cents;
        }
    }
    return -1;
}

// Returns total in cents, or -1 if invalid
ll calculate_total(const char* code, int qty) {
    if (qty < 1 || qty > 1000000) return -1;
    ll unit = get_price(code);
    if (unit < 0) return -1;
    return unit * (ll)qty;
}

static char hexval(char c) {
    if (c >= '0' && c <= '9') return (char)(c - '0');
    if (c >= 'A' && c <= 'F') return (char)(10 + c - 'A');
    if (c >= 'a' && c <= 'f') return (char)(10 + c - 'a');
    return (char)-1;
}

static void url_decode(char* dst, const char* src, size_t dstsz) {
    size_t di = 0;
    for (size_t i = 0; src[i] != '\0' && di + 1 < dstsz; ++i) {
        if (src[i] == '%' && src[i+1] && src[i+2]) {
            char h1 = hexval(src[i+1]);
            char h2 = hexval(src[i+2]);
            if (h1 >= 0 && h2 >= 0) {
                dst[di++] = (char)((h1 << 4) | h2);
                i += 2;
                continue;
            }
        } else if (src[i] == '+') {
            dst[di++] = ' ';
            continue;
        }
        dst[di++] = src[i];
    }
    dst[di] = '\0';
}

static int parse_query(const char* q, char* code_out, size_t code_sz, char* qty_out, size_t qty_sz) {
    code_out[0] = '\0';
    qty_out[0] = '\0';
    if (!q) return 0;
    const char* p = q;
    while (*p) {
        const char* amp = strchr(p, '&');
        size_t len = amp ? (size_t)(amp - p) : strlen(p);
        const char* eq = memchr(p, '=', len);
        if (eq) {
            size_t klen = (size_t)(eq - p);
            size_t vlen = len - klen - 1;
            char k[32] = {0}, v[64] = {0};
            size_t cpyk = klen < sizeof(k)-1 ? klen : sizeof(k)-1;
            size_t cpyv = vlen < sizeof(v)-1 ? vlen : sizeof(v)-1;
            memcpy(k, p, cpyk); k[cpyk] = '\0';
            memcpy(v, eq+1, cpyv); v[cpyv] = '\0';
            char kdec[32]; char vdec[64];
            url_decode(kdec, k, sizeof(kdec));
            url_decode(vdec, v, sizeof(vdec));
            if (strcmp(kdec, "code") == 0) {
                snprintf(code_out, code_sz, "%s", vdec);
            } else if (strcmp(kdec, "qty") == 0) {
                snprintf(qty_out, qty_sz, "%s", vdec);
            }
        }
        if (!amp) break;
        p = amp + 1;
    }
    return 1;
}

static void format_money(char* out, size_t sz, ll cents) {
    int neg = cents < 0;
    if (neg) cents = -cents;
    ll dollars = cents / 100;
    ll rem = cents % 100;
    snprintf(out, sz, "%s%lld.%02lld", neg ? "-" : "", dollars, rem);
}

static void send_response(int client, int status, const char* body) {
    const char* text = "OK";
    if (status == 400) text = "Bad Request";
    else if (status == 404) text = "Not Found";
    else if (status == 405) text = "Method Not Allowed";
    char header[512];
    int blen = (int)strlen(body);
    int hlen = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "X-Content-Type-Options: nosniff\r\n"
        "Cache-Control: no-store\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n", status, text, blen);
    send(client, header, (size_t)hlen, 0);
    send(client, body, (size_t)blen, 0);
}

static void handle_client(int client) {
    char buf[4096];
    memset(buf, 0, sizeof(buf));
    ssize_t n = recv(client, buf, sizeof(buf)-1, 0);
    if (n <= 0) {
        close(client);
        return;
    }
    // Parse request line
    char method[8]={0}, target[1024]={0}, version[16]={0};
    if (sscanf(buf, "%7s %1023s %15s", method, target, version) != 3) {
        send_response(client, 400, "{\"error\":\"Bad Request\"}");
        close(client);
        return;
    }
    if (strcmp(method, "GET") != 0) {
        send_response(client, 405, "{\"error\":\"Method Not Allowed\"}");
        close(client);
        return;
    }
    char path[1024]={0}, query[1024]={0};
    const char* qm = strchr(target, '?');
    if (qm) {
        size_t plen = (size_t)(qm - target);
        size_t cplen = plen < sizeof(path)-1 ? plen : sizeof(path)-1;
        memcpy(path, target, cplen); path[cplen] = '\0';
        snprintf(query, sizeof(query), "%s", qm+1);
    } else {
        snprintf(path, sizeof(path), "%s", target);
    }
    if (strcmp(path, "/price") != 0) {
        send_response(client, 404, "{\"error\":\"Not Found\"}");
        close(client);
        return;
    }
    char code[64], qty_s[32];
    parse_query(query, code, sizeof(code), qty_s, sizeof(qty_s));
    int qty = -1;
    if (qty_s[0] != '\0') {
        char* endp = NULL;
        long val = strtol(qty_s, &endp, 10);
        if (endp == qty_s || *endp != '\0' || val < 1 || val > 1000000) {
            send_response(client, 400, "{\"error\":\"Invalid qty\"}");
            close(client);
            return;
        }
        qty = (int)val;
    } else {
        send_response(client, 400, "{\"error\":\"Invalid qty\"}");
        close(client);
        return;
    }
    char codeU[64];
    snprintf(codeU, sizeof(codeU), "%s", code);
    for (size_t i = 0; codeU[i]; ++i) codeU[i] = (char)toupper((unsigned char)codeU[i]);
    if (!is_valid_code(codeU)) {
        send_response(client, 400, "{\"error\":\"Invalid or unknown code\"}");
        close(client);
        return;
    }
    ll unit = get_price(codeU);
    if (unit < 0) {
        send_response(client, 400, "{\"error\":\"Invalid or unknown code\"}");
        close(client);
        return;
    }
    ll total = unit * (ll)qty;
    char unit_s[64], total_s[64];
    format_money(unit_s, sizeof(unit_s), unit);
    format_money(total_s, sizeof(total_s), total);
    char body[512];
    snprintf(body, sizeof(body),
             "{\"code\":\"%s\",\"qty\":%d,\"unit_price\":\"%s\",\"total_price\":\"%s\"}",
             codeU, qty, unit_s, total_s);
    send_response(client, 200, body);
    close(client);
}

int main(void) {
    // 5 test cases
    char money[64];
    ll p = get_price("A100"); format_money(money, sizeof(money), p); printf("TEST get_price A100 -> %s\n", p>=0?money:"-1");
    ll t;
    t = calculate_total("A100", 2); format_money(money, sizeof(money), t); printf("TEST calculate_total A100 x2 -> %s\n", t>=0?money:"-1");
    t = calculate_total("B200", 5); format_money(money, sizeof(money), t); printf("TEST calculate_total B200 x5 -> %s\n", t>=0?money:"-1");
    t = calculate_total("C300", 1); format_money(money, sizeof(money), t); printf("TEST calculate_total C300 x1 -> %s\n", t>=0?money:"-1");
    t = calculate_total("Z999", 3); if (t>=0){format_money(money, sizeof(money), t); printf("TEST invalid code Z999 x3 -> %s\n", money);} else {printf("TEST invalid code Z999 x3 -> -1\n");}

    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);
    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(srv); return 1; }
    if (listen(srv, 10) < 0) { perror("listen"); close(srv); return 1; }
    printf("C server running at http://127.0.0.1:5000/price?code=A100&qty=2\n");
    while (1) {
        int client = accept(srv, NULL, NULL);
        if (client < 0) {
            if (errno == EINTR) continue;
            perror("accept"); break;
        }
        handle_client(client);
    }
    close(srv);
    return 0;
}