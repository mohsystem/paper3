#include <arpa/inet.h>
#include <netinet/in.h>
#include <regex.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 2048

static const char *ALLOW_RE = "(?i)^https?://[^[:space:]]*example\\.com[^[:space:]]*$";

int compile_regex(regex_t *re, const char *pattern, int cflags) {
    int rc = regcomp(re, pattern, cflags);
    return rc;
}

int regex_match(const char *pattern, const char *text) {
    regex_t re;
    int cflags = REG_EXTENDED | REG_NOSUB | REG_ICASE;
#ifdef REG_UTF8
    cflags |= REG_UTF8;
#endif
    if (compile_regex(&re, pattern, cflags) != 0) {
        return 0;
    }
    int rc = regexec(&re, text, 0, NULL, 0);
    regfree(&re);
    return rc == 0;
}

int contains_ctl(const char *s) {
    for (const unsigned char *p = (const unsigned char *)s; *p; ++p) {
        if (*p == '\r' || *p == '\n') return 1;
        if (*p < 0x20 && *p != '\t') return 1;
        if (*p == 0x7F) return 1;
    }
    return 0;
}

char *trim_copy(const char *s) {
    if (!s) return NULL;
    const char *start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char *end = s + strlen(s);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    size_t len = (size_t)(end - start);
    char *out = (char *)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

char hexval(char c) {
    if (c >= '0' && c <= '9') return (char)(c - '0');
    if (c >= 'a' && c <= 'f') return (char)(c - 'a' + 10);
    if (c >= 'A' && c <= 'F') return (char)(c - 'A' + 10);
    return (char)-1;
}

char *url_decode(const char *s) {
    size_t len = strlen(s);
    char *out = (char *)malloc(len + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        if (s[i] == '%' && i + 2 < len) {
            char h = hexval(s[i + 1]);
            char l = hexval(s[i + 2]);
            if (h >= 0 && l >= 0) {
                out[j++] = (char)(h * 16 + l);
                i += 2;
            } else {
                out[j++] = s[i];
            }
        } else if (s[i] == '+') {
            out[j++] = ' ';
        } else {
            out[j++] = s[i];
        }
    }
    out[j] = '\0';
    return out;
}

char *get_query_param(const char *query, const char *key) {
    if (!query) return NULL;
    size_t keylen = strlen(key);
    const char *p = query;
    while (*p) {
        const char *amp = strchr(p, '&');
        size_t seglen = amp ? (size_t)(amp - p) : strlen(p);
        const char *eq = memchr(p, '=', seglen);
        size_t klen = eq ? (size_t)(eq - p) : seglen;
        if (klen == keylen && strncmp(p, key, keylen) == 0) {
            const char *vstart = eq ? eq + 1 : p + klen;
            size_t vlen = seglen - (eq ? (size_t)(eq - p + 1) : klen);
            char *val = (char *)malloc(vlen + 1);
            if (!val) return NULL;
            memcpy(val, vstart, vlen);
            val[vlen] = '\0';
            char *dec = url_decode(val);
            free(val);
            return dec;
        }
        if (!amp) break;
        p = amp + 1;
    }
    return NULL;
}

char *evaluate_redirect(const char *target_in) {
    if (!target_in) {
        char *r = (char *)malloc(2);
        strcpy(r, "/");
        return r;
    }
    char *trimmed = trim_copy(target_in);
    if (!trimmed) {
        char *r = (char *)malloc(2);
        strcpy(r, "/");
        return r;
    }
    size_t len = strlen(trimmed);
    if (len == 0 || len > MAX_LEN || contains_ctl(trimmed) || !regex_match(ALLOW_RE, trimmed)) {
        free(trimmed);
        char *r = (char *)malloc(2);
        strcpy(r, "/");
        return r;
    }
    // Return a copy to satisfy "return output"
    char *ok = (char *)malloc(len + 1);
    strcpy(ok, trimmed);
    free(trimmed);
    return ok;
}

void send_response(int client_fd, const char *location) {
    char body[4096];
    snprintf(body, sizeof(body), "Redirecting to: %s\n", location);
    char header[4096];
    snprintf(header, sizeof(header),
             "HTTP/1.1 302 Found\r\n"
             "Location: %s\r\n"
             "Content-Type: text/plain; charset=utf-8\r\n"
             "Cache-Control: no-store\r\n"
             "Content-Security-Policy: default-src 'none'; frame-ancestors 'none'\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n"
             "\r\n",
             location, strlen(body));
    send(client_fd, header, strlen(header), 0);
    send(client_fd, body, strlen(body), 0);
}

void run_server_c() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return;
    }
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8083);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return;
    }
    if (listen(server_fd, 8) < 0) {
        perror("listen");
        close(server_fd);
        return;
    }
    printf("C server listening on http://127.0.0.1:8083/go?target=...\n");
    for (;;) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            break;
        }
        char buf[4096];
        ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) {
            close(client_fd);
            continue;
        }
        buf[n] = '\0';
        char *line_end = strstr(buf, "\r\n");
        if (!line_end) {
            close(client_fd);
            continue;
        }
        *line_end = '\0';
        char method[8], path_query[2048], proto[16];
        method[0] = path_query[0] = proto[0] = '\0';
        sscanf(buf, "%7s %2047s %15s", method, path_query, proto);
        char *path = path_query;
        char *query = NULL;
        char *qmark = strchr(path_query, '?');
        if (qmark) {
            *qmark = '\0';
            query = qmark + 1;
        }
        char *target = NULL;
        if (strcmp(path, "/go") == 0 && query) {
            target = get_query_param(query, "target");
        }
        char *location = evaluate_redirect(target ? target : "");
        send_response(client_fd, location);
        if (target) free(target);
        free(location);
        close(client_fd);
    }
    close(server_fd);
}

int main() {
    const char *tests[5] = {
        "https://example.com",
        "http://sub.example.com/path?x=1",
        "https://notexample.com",
        "https://example.com.evil.com/attack",
        "/local/path?target=https://example.com"
    };
    for (int i = 0; i < 5; ++i) {
        char *res = evaluate_redirect(tests[i]);
        printf("C Test %d: input=%s -> redirect=%s\n", i + 1, tests[i], res);
        free(res);
    }
    run_server_c();
    return 0;
}