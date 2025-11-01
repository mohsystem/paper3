#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8083
#define MAX_PARAM_LEN 2048
#define MAX_QUERY_LEN 4096
#define RECV_BUF 8192

static regex_t allow_re;
static int regex_inited = 0;

static void init_regex(void) {
    if (!regex_inited) {
        int rc = regcomp(&allow_re, "example\\.com", REG_ICASE | REG_EXTENDED);
        if (rc != 0) {
            fprintf(stderr, "regcomp failed\n");
            exit(1);
        }
        regex_inited = 1;
    }
}

static void cleanup_regex(void) {
    if (regex_inited) {
        regfree(&allow_re);
        regex_inited = 0;
    }
}

static void strip_crlf(char *s) {
    char *w = s;
    for (char *r = s; *r; ++r) {
        if (*r != '\r' && *r != '\n') {
            *w++ = *r;
        }
    }
    *w = '\0';
}

static int hexval(char ch) {
    if (ch >= '0' && ch <= '9') return ch - '0';
    if (ch >= 'a' && ch <= 'f') return 10 + (ch - 'a');
    if (ch >= 'A' && ch <= 'F') return 10 + (ch - 'A');
    return -1;
}

static void url_decode_limited(const char *in, char *out, size_t out_cap) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0'; ++i) {
        if (oi + 1 >= out_cap) break;
        unsigned char c = (unsigned char)in[i];
        if (c == '%' && in[i+1] && in[i+2]) {
            int hi = hexval(in[i+1]);
            int lo = hexval(in[i+2]);
            if (hi >= 0 && lo >= 0) {
                out[oi++] = (char)((hi << 4) | lo);
                i += 2;
                continue;
            }
        }
        if (c == '+') out[oi++] = ' ';
        else out[oi++] = (char)c;
    }
    out[oi] = '\0';
}

static const char* decide_redirect(const char *target, char *out, size_t out_cap) {
    if (target == NULL || target[0] == '\0') {
        return "/";
    }
    char buf[MAX_PARAM_LEN + 1];
    size_t len = strnlen(target, MAX_PARAM_LEN + 1);
    if (len > MAX_PARAM_LEN) {
        return "/";
    }
    memcpy(buf, target, len);
    buf[len] = '\0';
    strip_crlf(buf);
    if (regexec(&allow_re, buf, 0, NULL, 0) == 0) {
        // allowed
        if (out != NULL && out_cap > 0) {
            strncpy(out, buf, out_cap - 1);
            out[out_cap - 1] = '\0';
            return out;
        }
        return buf; // not used in this code path
    }
    return "/";
}

static void parse_target_param(const char *request_line, char *out, size_t out_cap) {
    // Initialize out as empty
    if (out_cap > 0) out[0] = '\0';
    if (!request_line) return;
    const char *sp1 = strchr(request_line, ' ');
    if (!sp1) return;
    const char *sp2 = strchr(sp1 + 1, ' ');
    if (!sp2) return;
    size_t pathq_len = (size_t)(sp2 - (sp1 + 1));
    if (pathq_len == 0 || pathq_len > MAX_QUERY_LEN + 1024) return;
    char *pathq = (char*)malloc(pathq_len + 1);
    if (!pathq) return;
    memcpy(pathq, sp1 + 1, pathq_len);
    pathq[pathq_len] = '\0';
    char *qmark = strchr(pathq, '?');
    if (!qmark) { free(pathq); return; }
    char *query = qmark + 1;
    if (strlen(query) > MAX_QUERY_LEN) { free(pathq); return; }
    char *p = query;
    while (p && *p) {
        char *amp = strchr(p, '&');
        char *kv_end = amp ? amp : (p + strlen(p));
        char *eq = memchr(p, '=', (size_t)(kv_end - p));
        if (eq) {
            size_t name_len = (size_t)(eq - p);
            if (name_len == 6 && strncmp(p, "target", 6) == 0) {
                size_t val_len = (size_t)(kv_end - (eq + 1));
                if (val_len > MAX_PARAM_LEN) val_len = MAX_PARAM_LEN;
                char raw[MAX_PARAM_LEN + 1];
                memcpy(raw, eq + 1, val_len);
                raw[val_len] = '\0';
                url_decode_limited(raw, out, out_cap);
                free(pathq);
                return;
            }
        }
        p = amp ? (amp + 1) : NULL;
    }
    free(pathq);
}

static void serve(void) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return;
    }
    int opt = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        perror("setsockopt");
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(PORT);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
        perror("bind");
        close(fd);
        return;
    }
    if (listen(fd, 8) != 0) {
        perror("listen");
        close(fd);
        return;
    }
    printf("C server listening on http://127.0.0.1:%d - try /?target=http://example.com\n", PORT);

    char recvbuf[RECV_BUF];
    for (;;) {
        int cfd = accept(fd, NULL, NULL);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }
        // Read request until header end or buffer full
        size_t total = 0;
        int header_done = 0;
        while (total < RECV_BUF) {
            ssize_t n = recv(cfd, recvbuf + total, RECV_BUF - total, 0);
            if (n <= 0) break;
            total += (size_t)n;
            if (total >= 4) {
                for (size_t i = 0; i + 3 < total; ++i) {
                    if (recvbuf[i] == '\r' && recvbuf[i+1] == '\n' && recvbuf[i+2] == '\r' && recvbuf[i+3] == '\n') {
                        header_done = 1;
                        break;
                    }
                }
                if (header_done) break;
            }
        }
        // Ensure null-terminated for parsing
        size_t term = (total < RECV_BUF) ? total : (RECV_BUF - 1);
        recvbuf[term] = '\0';

        // Get first line
        char *crlf = strstr(recvbuf, "\r\n");
        char firstLine[1024];
        firstLine[0] = '\0';
        if (crlf) {
            size_t flen = (size_t)(crlf - recvbuf);
            if (flen >= sizeof(firstLine)) flen = sizeof(firstLine) - 1;
            memcpy(firstLine, recvbuf, flen);
            firstLine[flen] = '\0';
        }
        char target_dec[MAX_PARAM_LEN + 1];
        target_dec[0] = '\0';
        parse_target_param(firstLine, target_dec, sizeof(target_dec));

        char redirect[MAX_PARAM_LEN + 1];
        const char *loc = decide_redirect(target_dec, redirect, sizeof(redirect));

        char resp[4096];
        int n = snprintf(resp, sizeof(resp),
                         "HTTP/1.1 302 Found\r\n"
                         "Location: %s\r\n"
                         "Content-Length: 0\r\n"
                         "Connection: close\r\n"
                         "\r\n",
                         loc);
        if (n < 0) n = 0;
        if (n > (int)sizeof(resp)) n = (int)sizeof(resp);
        send(cfd, resp, (size_t)n, 0);
        close(cfd);
    }
    close(fd);
}

int main(void) {
    signal(SIGPIPE, SIG_IGN);
    init_regex();
    // 5 test cases
    const char *tests[] = {
        "http://example.com/path",
        "https://sub.example.com/",
        "http://evil.com/?q=example.com",
        "/local/path",
        "javascript:example.com"
    };
    char out[MAX_PARAM_LEN + 1];
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        const char *res = decide_redirect(tests[i], out, sizeof(out));
        printf("Input: %s -> Redirect: %s\n", tests[i], res);
    }
    serve();
    cleanup_regex();
    return 0;
}