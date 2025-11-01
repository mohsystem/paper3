/* Chain-of-Through (concise):
 * 1) Create /redirect/<url> route, redirect only to example.com.
 * 2) Security: validate scheme/host, forbid CR/LF, bound lengths.
 * 3) Implement minimal HTTP server with careful parsing and headers.
 * 4) Review: use fixed buffers and length checks to avoid overflow.
 * 5) Output: final code with tests.
 */

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_URL_LEN 2048

typedef struct {
    int status;
    int redirect; // boolean
    char location[MAX_URL_LEN + 1];
    char body[256];
} Result;

static int has_crlf(const char* s) {
    for (; *s; ++s) {
        if (*s == '\r' || *s == '\n') return 1;
    }
    return 0;
}

static void tolower_inplace(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static void percent_decode(const char* in, char* out, size_t outsz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0' && oi + 1 < outsz; ++i) {
        if (in[i] == '%' && in[i+1] && in[i+2]) {
            char h1 = in[i+1], h2 = in[i+2];
            int v = -1;
            if (isxdigit((unsigned char)h1) && isxdigit((unsigned char)h2)) {
                int hi = (h1 <= '9') ? (h1 - '0') : (toupper((unsigned char)h1) - 'A' + 10);
                int lo = (h2 <= '9') ? (h2 - '0') : (toupper((unsigned char)h2) - 'A' + 10);
                v = (hi << 4) | lo;
            }
            if (v >= 0) {
                out[oi++] = (char)v;
                i += 2;
            } else {
                out[oi++] = in[i];
            }
        } else {
            out[oi++] = in[i];
        }
    }
    out[oi] = '\0';
}

static int extract_scheme_host(const char* url, char* scheme, size_t schm_sz, char* host, size_t host_sz) {
    scheme[0] = '\0';
    host[0] = '\0';
    const char* p = strstr(url, "://");
    if (!p) return 0;
    size_t slen = (size_t)(p - url);
    if (slen == 0 || slen >= schm_sz) return 0;
    memcpy(scheme, url, slen);
    scheme[slen] = '\0';
    char* schml = scheme;
    tolower_inplace(schml);
    const char* rest = p + 3;
    // Strip userinfo if any
    const char* at = strrchr(rest, '@');
    if (at) rest = at + 1;
    // authority until / ? #
    size_t alen = strcspn(rest, "/?#");
    if (alen == 0) return 0;
    if (alen >= host_sz) alen = host_sz - 1;
    // extract host (without port)
    const char* authority = rest;
    if (authority[0] == '[') {
        // IPv6 literal not allowed; fail
        return 0;
    } else {
        const char* colon = memchr(authority, ':', alen);
        size_t hlen = colon ? (size_t)(colon - authority) : alen;
        if (hlen == 0 || hlen >= host_sz) return 0;
        memcpy(host, authority, hlen);
        host[hlen] = '\0';
    }
    tolower_inplace(host);
    return 1;
}

Result redirect_invalid_url(const char* input_url) {
    Result r;
    r.status = 400; r.redirect = 0; r.location[0] = '\0';
    snprintf(r.body, sizeof(r.body), "%s", "Invalid or disallowed URL");
    if (!input_url) return r;
    size_t len = strlen(input_url);
    if (len == 0 || len > MAX_URL_LEN || has_crlf(input_url)) return r;

    char scheme[16], host[256];
    if (!extract_scheme_host(input_url, scheme, sizeof(scheme), host, sizeof(host))) return r;
    if (strcmp(scheme, "http") != 0 && strcmp(scheme, "https") != 0) return r;
    if (strcmp(host, "example.com") != 0) return r;
    if (has_crlf(input_url)) return r;

    r.status = 302;
    r.redirect = 1;
    snprintf(r.location, sizeof(r.location), "%s", input_url);
    snprintf(r.body, sizeof(r.body), "%s", "Redirecting...");
    return r;
}

static void send_response(int client_fd, const Result* res) {
    char header[1024];
    const char* status_text = (res->status == 302) ? "Found" : (res->status == 400 ? "Bad Request" : "OK");
    int n = snprintf(header, sizeof(header),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/plain; charset=utf-8\r\n"
                     "%s%s%s"
                     "Content-Length: %zu\r\n"
                     "Connection: close\r\n\r\n",
                     res->status, status_text,
                     res->redirect && res->location[0] ? "Location: " : "",
                     res->redirect && res->location[0] ? res->location : "",
                     res->redirect && res->location[0] ? "\r\n" : "",
                     strlen(res->body));
    send(client_fd, header, (size_t)n, 0);
    send(client_fd, res->body, strlen(res->body), 0);
}

static void run_server() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return; }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8083);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(server_fd); return; }
    if (listen(server_fd, 10) < 0) { perror("listen"); close(server_fd); return; }
    printf("C server listening on http://127.0.0.1:8083\n");
    for (;;) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) { perror("accept"); break; }
        char buf[4096];
        ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { close(client_fd); continue; }
        buf[n] = '\0';
        // Parse request line
        char method[8] = {0}, target[2048] = {0};
        if (sscanf(buf, "%7s %2047s", method, target) != 2) {
            Result r = {400, 0, "", "Invalid or disallowed URL"};
            send_response(client_fd, &r);
            close(client_fd);
            continue;
        }
        if (strcmp(method, "GET") != 0) {
            Result r = {405, 0, "", "Method Not Allowed"};
            send_response(client_fd, &r);
            close(client_fd);
            continue;
        }
        const char* prefix = "/redirect/";
        Result res;
        if (strcmp(target, "/redirect") == 0) {
            res = redirect_invalid_url("");
        } else if (strncmp(target, prefix, strlen(prefix)) == 0) {
            char encoded[MAX_URL_LEN + 1];
            snprintf(encoded, sizeof(encoded), "%s", target + strlen(prefix));
            char decoded[MAX_URL_LEN + 1];
            percent_decode(encoded, decoded, sizeof(decoded));
            res = redirect_invalid_url(decoded);
        } else {
            Result r404 = {404, 0, "", "Not Found"};
            send_response(client_fd, &r404);
            close(client_fd);
            continue;
        }
        send_response(client_fd, &res);
        close(client_fd);
    }
    close(server_fd);
}

int main(void) {
    // 5 test cases
    const char* tests[5] = {
        "https://example.com/path?q=1",
        "http://example.com",
        "https://notexample.com/bad",
        "javascript:alert(1)",
        "https://example.com:443/ok#frag"
    };
    for (int i = 0; i < 5; ++i) {
        Result r = redirect_invalid_url(tests[i]);
        printf("Test: %s => status=%d, redirect=%s", tests[i], r.status, r.redirect ? "true" : "false");
        if (r.redirect) printf(", location=%s", r.location);
        printf("\n");
    }
    run_server();
    return 0;
}