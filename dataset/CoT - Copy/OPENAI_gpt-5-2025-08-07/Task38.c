#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PORT 8083
#define MAX_BODY 4096
#define MAX_MSG 200

typedef struct {
    char sid[65];
    char token[129];
} session_t;

#define MAX_SESS 32
static session_t SESS[MAX_SESS];

static void trim(char *s) {
    size_t len = strlen(s);
    size_t a = 0;
    while (a < len && (s[a] == ' ' || s[a] == '\t')) a++;
    size_t b = len;
    while (b > a && (s[b-1] == ' ' || s[b-1] == '\t' || s[b-1] == '\r' || s[b-1] == '\n')) b--;
    if (a > 0 || b < len) memmove(s, s + a, b - a);
    s[b - a] = '\0';
}

static void hexbytes(const unsigned char *in, size_t n, char *out, size_t outlen) {
    static const char *hex = "0123456789abcdef";
    size_t j = 0;
    for (size_t i = 0; i < n && j + 2 < outlen; i++) {
        out[j++] = hex[(in[i] >> 4) & 0xF];
        out[j++] = hex[in[i] & 0xF];
    }
    out[j] = '\0';
}

static void gen_token(size_t bytes, char *out, size_t outlen) {
    unsigned char buf[128];
    if (bytes > sizeof(buf)) bytes = sizeof(buf);
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd >= 0) {
        ssize_t r = read(fd, buf, bytes);
        close(fd);
        if (r != (ssize_t)bytes) {
            for (size_t i = 0; i < bytes; i++) buf[i] = (unsigned char)rand();
        }
    } else {
        for (size_t i = 0; i < bytes; i++) buf[i] = (unsigned char)rand();
    }
    hexbytes(buf, bytes, out, outlen);
}

static const char *csp_header() {
    return "default-src 'none'; style-src 'self' 'unsafe-inline'; script-src 'none'; img-src 'self'; base-uri 'none'; form-action 'self'";
}

static void html_escape(const char *in, char *out, size_t outlen) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 6 < outlen; i++) {
        char c = in[i];
        switch (c) {
            case '&': j += snprintf(out + j, outlen - j, "&amp;"); break;
            case '<': j += snprintf(out + j, outlen - j, "&lt;"); break;
            case '>': j += snprintf(out + j, outlen - j, "&gt;"); break;
            case '"': j += snprintf(out + j, outlen - j, "&quot;"); break;
            case '\'': j += snprintf(out + j, outlen - j, "&#x27;"); break;
            case '/': j += snprintf(out + j, outlen - j, "&#x2F;"); break;
            default: out[j++] = c; out[j] = '\0';
        }
    }
    out[j] = '\0';
}

static bool validate_message(const char *raw) {
    if (!raw) return false;
    size_t len = strlen(raw);
    if (len == 0 || len > MAX_MSG) return false;
    const char *allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,;:!?@#()_-'\"";
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)raw[i];
        if (c == '\r' || c == '\n' || c == '\t') return false;
        const char *p = strchr(allowed, (char)c);
        if (!p) return false;
    }
    return true;
}

static char from_hex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static void url_decode(const char *in, char *out, size_t outlen) {
    size_t j = 0;
    for (size_t i = 0; in[i] && j + 1 < outlen; i++) {
        char c = in[i];
        if (c == '+') {
            out[j++] = ' ';
        } else if (c == '%' && in[i+1] && in[i+2]) {
            char h1 = from_hex(in[i+1]);
            char h2 = from_hex(in[i+2]);
            if (h1 >= 0 && h2 >= 0) {
                out[j++] = (char)((h1<<4)|h2);
                i += 2;
            } else {
                out[j++] = c;
            }
        } else {
            out[j++] = c;
        }
    }
    out[j] = '\0';
}

typedef struct {
    char key[64];
    char val[512];
} kv_t;

static size_t parse_form(const char *body, kv_t *out, size_t maxkv) {
    size_t count = 0;
    const char *p = body;
    while (*p && count < maxkv) {
        const char *amp = strchr(p, '&');
        size_t len = amp ? (size_t)(amp - p) : strlen(p);
        const char *eq = memchr(p, '=', len);
        char k[64] = {0}, v[512] = {0}, kd[64] = {0}, vd[512] = {0};
        if (eq) {
            size_t kl = (size_t)(eq - p);
            size_t vl = len - kl - 1;
            if (kl >= sizeof(k)) kl = sizeof(k)-1;
            if (vl >= sizeof(v)) vl = sizeof(v)-1;
            memcpy(k, p, kl); k[kl] = 0;
            memcpy(v, eq+1, vl); v[vl] = 0;
        } else {
            size_t kl = len;
            if (kl >= sizeof(k)) kl = sizeof(k)-1;
            memcpy(k, p, kl); k[kl] = 0;
            v[0] = 0;
        }
        url_decode(k, kd, sizeof(kd));
        url_decode(v, vd, sizeof(vd));
        strncpy(out[count].key, kd, sizeof(out[count].key)-1);
        strncpy(out[count].val, vd, sizeof(out[count].val)-1);
        count++;
        if (!amp) break;
        p = amp + 1;
    }
    return count;
}

static bool const_time_eq(const char *a, const char *b) {
    if (!a || !b) return false;
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return false;
    unsigned char r = 0;
    for (size_t i = 0; i < la; i++) r |= (unsigned char)a[i] ^ (unsigned char)b[i];
    return r == 0;
}

static session_t* sess_find(const char *sid) {
    for (int i = 0; i < MAX_SESS; i++) {
        if (SESS[i].sid[0] && strcmp(SESS[i].sid, sid) == 0) return &SESS[i];
    }
    return NULL;
}

static session_t* sess_get_or_create(const char *sid) {
    session_t *s = sess_find(sid);
    if (s) return s;
    for (int i = 0; i < MAX_SESS; i++) {
        if (!SESS[i].sid[0]) {
            strncpy(SESS[i].sid, sid, sizeof(SESS[i].sid)-1);
            return &SESS[i];
        }
    }
    // overwrite first if full
    strncpy(SESS[0].sid, sid, sizeof(SESS[0].sid)-1);
    return &SESS[0];
}

static void send_all(int fd, const char *data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, data + sent, len - sent, 0);
        if (n <= 0) break;
        sent += (size_t)n;
    }
}

static void respond_html(int cfd, int code, const char *status, const char *sid, const char *body) {
    char header[1024];
    int n = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "X-Content-Type-Options: nosniff\r\n"
        "Referrer-Policy: no-referrer\r\n"
        "X-Frame-Options: DENY\r\n"
        "Content-Security-Policy: %s\r\n"
        "Set-Cookie: sid=%s; HttpOnly; Path=/; SameSite=Strict\r\n"
        "Content-Length: %zu\r\n\r\n",
        code, status, csp_header(), sid ? sid : "", strlen(body));
    send_all(cfd, header, (size_t)n);
    send_all(cfd, body, strlen(body));
}

static void page_form(char *out, size_t outlen, const char *csrf) {
    snprintf(out, outlen,
        "<!doctype html><html><head><meta charset=\"utf-8\">"
        "<meta http-equiv=\"Content-Security-Policy\" content=\"%s\">"
        "<title>Echo</title></head><body>"
        "<h1>Secure Echo</h1>"
        "<form method=\"POST\" action=\"/echo\">"
        "<label>Message: <input type=\"text\" name=\"message\" maxlength=\"%d\"></label>"
        "<input type=\"hidden\" name=\"csrf\" value=\"%s\">"
        "<button type=\"submit\">Send</button>"
        "</form></body></html>",
        csp_header(), MAX_MSG, csrf);
}

static void page_msg(char *out, size_t outlen, const char *msg) {
    char esc[1024];
    html_escape(msg, esc, sizeof(esc));
    snprintf(out, outlen,
        "<!doctype html><html><head><meta charset=\"utf-8\">"
        "<meta http-equiv=\"Content-Security-Policy\" content=\"%s\">"
        "<title>Echo</title></head><body><p>%s</p>"
        "<p><a href=\"/\">Back</a></p></body></html>",
        csp_header(), esc);
}

static const char* get_header_line(const char *headers, const char *name, char *out, size_t outlen) {
    size_t namelen = strlen(name);
    const char *p = headers;
    while (*p) {
        const char *q = strstr(p, "\r\n");
        size_t len = q ? (size_t)(q - p) : strlen(p);
        if (len == 0) break;
        if (len > namelen && strncasecmp(p, name, namelen) == 0 && p[namelen] == ':') {
            size_t vl = len - namelen - 1;
            if (vl >= outlen) vl = outlen - 1;
            memcpy(out, p + namelen + 1, vl);
            out[vl] = 0;
            trim(out);
            return out;
        }
        if (!q) break;
        p = q + 2;
    }
    if (out && outlen) out[0] = 0;
    return NULL;
}

static void get_cookie_sid(const char *cookie, char *sid, size_t sidlen) {
    sid[0] = 0;
    if (!cookie) return;
    char cpy[512];
    strncpy(cpy, cookie, sizeof(cpy)-1);
    cpy[sizeof(cpy)-1] = 0;
    char *save = NULL;
    char *tok = strtok_r(cpy, ";", &save);
    while (tok) {
        trim(tok);
        char *eq = strchr(tok, '=');
        if (eq) {
            *eq = 0;
            char *k = tok;
            char *v = eq + 1;
            trim(k); trim(v);
            if (strcmp(k, "sid") == 0) {
                strncpy(sid, v, sidlen-1);
                sid[sidlen-1] = 0;
                return;
            }
        }
        tok = strtok_r(NULL, ";", &save);
    }
}

static void handle_client(int cfd) {
    char buf[8192];
    ssize_t n = recv(cfd, buf, sizeof(buf)-1, 0);
    if (n <= 0) return;
    buf[n] = 0;
    char *header_end = strstr(buf, "\r\n\r\n");
    if (!header_end) return;
    size_t header_len = (size_t)(header_end - buf);
    char start_line[512] = {0};
    char *crlf = strstr(buf, "\r\n");
    if (!crlf) return;
    size_t slen = (size_t)(crlf - buf);
    if (slen >= sizeof(start_line)) slen = sizeof(start_line)-1;
    memcpy(start_line, buf, slen);
    start_line[slen] = 0;

    char method[8]={0}, path[256]={0}, version[16]={0};
    sscanf(start_line, "%7s %255s %15s", method, path, version);

    char headers_str[4096]={0};
    size_t hcopy = header_len - slen - 2;
    if (hcopy >= sizeof(headers_str)) hcopy = sizeof(headers_str)-1;
    memcpy(headers_str, crlf+2, hcopy);
    headers_str[hcopy] = 0;

    char cookie[512]={0};
    get_header_line(headers_str, "Cookie", cookie, sizeof(cookie));

    char sid[65]={0};
    get_cookie_sid(cookie, sid, sizeof(sid));
    if (strlen(sid) < 16) {
        gen_token(16, sid, sizeof(sid));
    }

    if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        session_t *s = sess_get_or_create(sid);
        gen_token(32, s->token, sizeof(s->token));
        char body[2048];
        page_form(body, sizeof(body), s->token);
        respond_html(cfd, 200, "OK", sid, body);
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/echo") == 0) {
        char ctype[256]={0}, clen[64]={0};
        get_header_line(headers_str, "Content-Type", ctype, sizeof(ctype));
        get_header_line(headers_str, "Content-Length", clen, sizeof(clen));
        size_t content_length = (size_t)strtoul(clen, NULL, 10);
        if (strncasecmp(ctype, "application/x-www-form-urlencoded", 33) != 0 || content_length > MAX_BODY) {
            char body[512]; page_msg(body, sizeof(body), "Invalid request.");
            respond_html(cfd, 400, "Bad Request", sid, body);
            return;
        }
        char *bodyStart = header_end + 4;
        size_t have = (size_t)(buf + n - bodyStart);
        char bodybuf[MAX_BODY+1];
        size_t to_copy = have > content_length ? content_length : have;
        memcpy(bodybuf, bodyStart, to_copy);
        size_t total = to_copy;
        while (total < content_length && total < MAX_BODY) {
            ssize_t r = recv(cfd, bodybuf + total, content_length - total, 0);
            if (r <= 0) break;
            total += (size_t)r;
        }
        if (total > MAX_BODY) {
            char body[512]; page_msg(body, sizeof(body), "Payload too large.");
            respond_html(cfd, 413, "Payload Too Large", sid, body);
            return;
        }
        bodybuf[total] = 0;
        kv_t kv[16];
        size_t kvn = parse_form(bodybuf, kv, 16);
        char msg[512]="", csrf[256]="";
        for (size_t i = 0; i < kvn; i++) {
            if (strcmp(kv[i].key, "message") == 0) strncpy(msg, kv[i].val, sizeof(msg)-1);
            else if (strcmp(kv[i].key, "csrf") == 0) strncpy(csrf, kv[i].val, sizeof(csrf)-1);
        }
        session_t *s = sess_find(sid);
        if (!s || !const_time_eq(s->token, csrf)) {
            char body[512]; page_msg(body, sizeof(body), "Invalid CSRF token.");
            respond_html(cfd, 400, "Bad Request", sid, body);
            return;
        }
        if (!validate_message(msg)) {
            char body[512]; page_msg(body, sizeof(body), "Invalid input. Only basic punctuation and letters/numbers allowed.");
            respond_html(cfd, 400, "Bad Request", sid, body);
            return;
        }
        char esc[1024]; html_escape(msg, esc, sizeof(esc));
        char out[2048];
        snprintf(out, sizeof(out),
            "<!doctype html><html><head><meta charset=\"utf-8\">"
            "<meta http-equiv=\"Content-Security-Policy\" content=\"%s\">"
            "<title>Echo</title></head><body><h1>Echo result</h1>"
            "<p>You said: <strong>%s</strong></p><p><a href=\"/\">Back</a></p></body></html>",
            csp_header(), esc);
        respond_html(cfd, 200, "OK", sid, out);
    } else {
        char body[512]; page_msg(body, sizeof(body), "Not found.");
        respond_html(cfd, 404, "Not Found", sid, body);
    }
}

static void run_tests() {
    const char *tests[5] = {
        "<script>alert(1)</script>",
        "Hello, World!",
        "This_is-OK() #1!?",
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA", // 200 As
        "Emoji 🙂"
    };
    for (int i = 0; i < 5; i++) {
        char esc[512]; html_escape(tests[i], esc, sizeof(esc));
        printf("Test: %.30s%s => %s\n", tests[i], strlen(tests[i])>30?"...":"", validate_message(tests[i]) ? "OK" : "INVALID");
    }
}

int main() {
    signal(SIGPIPE, SIG_IGN);
    srand((unsigned)time(NULL));
    run_tests();

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }
    int opt = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr; memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); close(fd); return 1; }
    if (listen(fd, 16) < 0) { perror("listen"); close(fd); return 1; }
    printf("C server running on http://localhost:%d/\n", PORT);
    while (1) {
        int c = accept(fd, NULL, NULL);
        if (c < 0) {
            if (errno == EINTR) continue;
            perror("accept"); break;
        }
        handle_client(c);
        close(c);
    }
    close(fd);
    return 0;
}