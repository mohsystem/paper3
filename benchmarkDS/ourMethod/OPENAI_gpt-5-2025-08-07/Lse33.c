#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef __unix__
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#endif

#define MAX_URL_LENGTH 2048
#define MAX_QUERY_LENGTH 4096

typedef struct {
    int statusCode;       // 302 or 400
    char location[MAX_URL_LENGTH + 1]; // when 302
    char body[64];        // when 400
} RedirectResultC;

static void strlower(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static void safe_copy(char* dst, size_t dstsz, const char* src) {
    if (dstsz == 0) return;
    size_t n = strlen(src);
    if (n >= dstsz) n = dstsz - 1;
    memcpy(dst, src, n);
    dst[n] = '\0';
}

static int starts_with_icase(const char* s, const char* prefix) {
    size_t ls = strlen(s), lp = strlen(prefix);
    if (ls < lp) return 0;
    for (size_t i = 0; i < lp; ++i) {
        if (tolower((unsigned char)s[i]) != tolower((unsigned char)prefix[i])) return 0;
    }
    return 1;
}

static void url_decode_inplace(char* s) {
    size_t len = strlen(s);
    size_t w = 0;
    for (size_t r = 0; r < len; ++r) {
        if (s[r] == '%' && r + 2 < len) {
            char h1 = s[r+1], h2 = s[r+2];
            int v1 = (h1 >= '0' && h1 <= '9') ? h1 - '0' :
                     (h1 >= 'A' && h1 <= 'F') ? 10 + (h1 - 'A') :
                     (h1 >= 'a' && h1 <= 'f') ? 10 + (h1 - 'a') : -1;
            int v2 = (h2 >= '0' && h2 <= '9') ? h2 - '0' :
                     (h2 >= 'A' && h2 <= 'F') ? 10 + (h2 - 'A') :
                     (h2 >= 'a' && h2 <= 'f') ? 10 + (h2 - 'a') : -1;
            if (v1 >= 0 && v2 >= 0) {
                s[w++] = (char)(v1 * 16 + v2);
                r += 2;
            } else {
                s[w++] = s[r];
            }
        } else if (s[r] == '+') {
            s[w++] = ' ';
        } else {
            s[w++] = s[r];
        }
    }
    s[w] = '\0';
}

RedirectResultC redirect_invalid_url(const char* url) {
    RedirectResultC res;
    res.statusCode = 400;
    res.location[0] = '\0';
    safe_copy(res.body, sizeof(res.body), "Invalid URL");

    if (url == NULL) return res;
    // Trim leading/trailing whitespace
    const char* start = url;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = url + strlen(url);
    while (end > start && isspace((unsigned char)end[-1])) end--;
    size_t len = (size_t)(end - start);
    if (len == 0 || len > MAX_URL_LENGTH) return res;

    // Copy trimmed into buffer
    char buf[MAX_URL_LENGTH + 1];
    if (len > MAX_URL_LENGTH) len = MAX_URL_LENGTH;
    memcpy(buf, start, len);
    buf[len] = '\0';

    // Reject CRLF
    for (size_t i = 0; i < len; ++i) {
        if (buf[i] == '\r' || buf[i] == '\n') {
            return res;
        }
    }

    if (!(starts_with_icase(buf, "http://") || starts_with_icase(buf, "https://"))) {
        return res;
    }
    char* scheme_sep = strstr(buf, "://");
    if (!scheme_sep) return res;
    char* host_start = scheme_sep + 3;
    if (*host_start == '\0') return res;

    // Find authority end
    char* p = host_start;
    while (*p && *p != '/' && *p != '?' && *p != '#') p++;
    size_t auth_len = (size_t)(p - host_start);
    if (auth_len == 0 || auth_len > 255) return res;

    // Copy authority
    char authority[256];
    if (auth_len >= sizeof(authority)) return res;
    memcpy(authority, host_start, auth_len);
    authority[auth_len] = '\0';

    if (strchr(authority, '@') != NULL) return res;

    // Extract host (strip port)
    char host[256];
    const char* colon = strchr(authority, ':');
    if (colon) {
        size_t hostlen = (size_t)(colon - authority);
        if (hostlen >= sizeof(host)) hostlen = sizeof(host) - 1;
        memcpy(host, authority, hostlen);
        host[hostlen] = '\0';
    } else {
        safe_copy(host, sizeof(host), authority);
    }
    // Lowercase host for compare
    for (char* q = host; *q; ++q) *q = (char)tolower((unsigned char)*q);
    if (strcmp(host, "example.com") != 0) {
        return res;
    }

    // Sanitize location: copy without CR/LF
    size_t w = 0;
    for (size_t i = 0; i < len && w < MAX_URL_LENGTH; ++i) {
        if (buf[i] != '\r' && buf[i] != '\n') {
            res.location[w++] = buf[i];
        }
    }
    res.location[w] = '\0';
    res.statusCode = 302;
    res.body[0] = '\0';
    return res;
}

#ifdef __unix__
static void send_all(int fd, const char* data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, data + sent, len - sent, 0);
        if (n <= 0) break;
        sent += (size_t)n;
    }
}

static void run_server(unsigned short port) {
    int srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) {
        perror("socket");
        return;
    }
    int opt = 1;
    setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);
    if (bind(srv, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("bind");
        close(srv);
        return;
    }
    if (listen(srv, 5) != 0) {
        perror("listen");
        close(srv);
        return;
    }
    printf("Server on http://127.0.0.1:%u/go?url=http%%3A%%2F%%2Fexample.com%%2F\n", port);
    for (;;) {
        int cfd = accept(srv, NULL, NULL);
        if (cfd < 0) break;
        char buf[4096];
        ssize_t n = recv(cfd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { close(cfd); continue; }
        buf[n] = '\0';
        // Parse request line
        char* line_end = strstr(buf, "\r\n");
        if (!line_end) { close(cfd); continue; }
        *line_end = '\0';
        char method[8], target[2048], version[16];
        method[0] = target[0] = version[0] = '\0';
        sscanf(buf, "%7s %2047s %15s", method, target, version);
        if (strcasecmp(method, "GET") != 0) {
            const char* resp = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
            send_all(cfd, resp, strlen(resp));
            close(cfd);
            continue;
        }
        // Parse path and query
        char* qpos = strchr(target, '?');
        char path[2048];
        char query[2048];
        path[0] = '\0'; query[0] = '\0';
        if (qpos) {
            size_t plen = (size_t)(qpos - target);
            if (plen >= sizeof(path)) plen = sizeof(path) - 1;
            memcpy(path, target, plen);
            path[plen] = '\0';
            safe_copy(query, sizeof(query), qpos + 1);
        } else {
            safe_copy(path, sizeof(path), target);
        }
        if (strcmp(path, "/go") != 0 || query[0] == '\0' || strlen(query) > MAX_QUERY_LENGTH) {
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
            send_all(cfd, resp, strlen(resp));
            close(cfd);
            continue;
        }
        // Find url param
        char urlparam[MAX_URL_LENGTH + 1];
        urlparam[0] = '\0';
        char qcopy[2048];
        safe_copy(qcopy, sizeof(qcopy), query);
        char* saveptr = NULL;
        char* tok = strtok_r(qcopy, "&", &saveptr);
        while (tok) {
            char* eq = strchr(tok, '=');
            if (eq) {
                *eq = '\0';
                const char* key = tok;
                const char* val = eq + 1;
                if (strcmp(key, "url") == 0) {
                    safe_copy(urlparam, sizeof(urlparam), val);
                    break;
                }
            } else {
                if (strcmp(tok, "url") == 0) {
                    urlparam[0] = '\0';
                    break;
                }
            }
            tok = strtok_r(NULL, "&", &saveptr);
        }
        if (urlparam[0] == '\0' && strstr(query, "url=") == NULL) {
            const char* resp = "HTTP/1.1 400 Bad Request\r\nContent-Length: 0\r\n\r\n";
            send_all(cfd, resp, strlen(resp));
            close(cfd);
            continue;
        }
        url_decode_inplace(urlparam);
        RedirectResultC rr = redirect_invalid_url(urlparam);
        if (rr.statusCode == 302) {
            char resp[4096];
            int m = snprintf(resp, sizeof(resp),
                             "HTTP/1.1 302 Found\r\nLocation: %s\r\nContent-Length: 0\r\n\r\n",
                             rr.location);
            if (m < 0) m = 0;
            if ((size_t)m > sizeof(resp)) m = (int)sizeof(resp);
            send_all(cfd, resp, (size_t)m);
        } else {
            const char* body = "Invalid URL";
            char resp[512];
            int m = snprintf(resp, sizeof(resp),
                             "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\n\r\n%s",
                             strlen(body), body);
            if (m < 0) m = 0;
            if ((size_t)m > sizeof(resp)) m = (int)sizeof(resp);
            send_all(cfd, resp, (size_t)m);
        }
        close(cfd);
    }
    close(srv);
}
#endif

int main(int argc, char** argv) {
    // 5 test cases
    const char* tests[5] = {
        "http://example.com/",
        "https://example.com/path?x=1",
        "https://sub.example.com/",
        "ftp://example.com/",
        "https://example.com/\r\nX: a"
    };
    for (int i = 0; i < 5; ++i) {
        RedirectResultC r = redirect_invalid_url(tests[i]);
        if (r.statusCode == 302) {
            printf("OK 302 -> %s\n", r.location);
        } else {
            printf("ERR %d\n", r.statusCode);
        }
    }

    // Optionally start server: ./a.out serve
#ifdef __unix__
    if (argc > 1 && strcasecmp(argv[1], "serve") == 0) {
        run_server(8083);
    }
#endif
    return 0;
}