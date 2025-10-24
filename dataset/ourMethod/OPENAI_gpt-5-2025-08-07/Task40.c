#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_REQ 16384
#define MAX_BODY 8192
#define MAX_SESSIONS 256

typedef struct {
    char id[33];   // 16 bytes hex -> 32 chars + null
    char csrf[65]; // 32 bytes hex -> 64 chars + null
    int in_use;
} Session;

static Session g_sessions[MAX_SESSIONS];

static void secure_random_bytes(uint8_t* out, size_t len) {
    FILE* f = fopen("/dev/urandom", "rb");
    if (!f) {
        // fallback to rand() is not secure, but try to exit if /dev/urandom not available
        for (size_t i = 0; i < len; ++i) out[i] = (uint8_t)(i * 37u + 123u);
        return;
    }
    size_t r = fread(out, 1, len, f);
    (void)r;
    fclose(f);
}

static void hex_encode(const uint8_t* in, size_t len, char* out, size_t out_sz) {
    static const char* hex = "0123456789abcdef";
    if (out_sz < len * 2 + 1) return;
    for (size_t i = 0; i < len; ++i) {
        out[i * 2] = hex[in[i] >> 4];
        out[i * 2 + 1] = hex[in[i] & 0x0F];
    }
    out[len * 2] = '\0';
}

static void generate_token_hex(size_t bytes, char* out, size_t out_sz) {
    if (bytes == 0 || bytes > 1024) bytes = 32;
    uint8_t* buf = (uint8_t*)malloc(bytes);
    if (!buf) {
        if (out_sz > 1) out[0] = '\0';
        return;
    }
    secure_random_bytes(buf, bytes);
    hex_encode(buf, bytes, out, out_sz);
    memset(buf, 0, bytes);
    free(buf);
}

static bool const_time_eq(const char* a, const char* b) {
    size_t la = strlen(a), lb = strlen(b);
    if (la != lb) return false;
    uint8_t diff = 0;
    for (size_t i = 0; i < la; ++i) diff |= ((uint8_t)a[i]) ^ ((uint8_t)b[i]);
    return diff == 0;
}

static int hex_val(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static bool url_decode(const char* in, char* out, size_t out_sz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0'; ++i) {
        char c = in[i];
        if (c == '+') {
            if (oi + 1 >= out_sz) return false;
            out[oi++] = ' ';
        } else if (c == '%' && in[i + 1] && in[i + 2]) {
            int hi = hex_val(in[i + 1]);
            int lo = hex_val(in[i + 2]);
            if (hi < 0 || lo < 0) return false;
            if (oi + 1 >= out_sz) return false;
            out[oi++] = (char)((hi << 4) | lo);
            i += 2;
        } else {
            if (oi + 1 >= out_sz) return false;
            out[oi++] = c;
        }
    }
    if (oi >= out_sz) return false;
    out[oi] = '\0';
    return true;
}

static void html_escape(const char* in, char* out, size_t out_sz) {
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0'; ++i) {
        const char* rep = NULL;
        char buf[2] = {0, 0};
        switch (in[i]) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#39;"; break;
            default: buf[0] = in[i]; rep = buf; break;
        }
        for (size_t j = 0; rep[j] != '\0'; ++j) {
            if (oi + 1 >= out_sz) { out[oi] = '\0'; return; }
            out[oi++] = rep[j];
        }
    }
    out[oi] = '\0';
}

typedef struct {
    char key[65];
    char value[1025];
} Pair;

typedef struct {
    Pair items[32];
    size_t count;
} Form;

static void parse_form_urlencoded(const char* body, Form* form) {
    form->count = 0;
    const char* p = body;
    while (*p && form->count < 32) {
        const char* amp = strchr(p, '&');
        size_t len = amp ? (size_t)(amp - p) : strlen(p);
        char pair[1200];
        if (len >= sizeof(pair)) len = sizeof(pair) - 1;
        memcpy(pair, p, len);
        pair[len] = '\0';

        char* eq = strchr(pair, '=');
        char k_enc[400] = {0};
        char v_enc[1025] = {0};
        if (eq) {
            size_t klen = (size_t)(eq - pair);
            if (klen >= sizeof(k_enc)) klen = sizeof(k_enc) - 1;
            memcpy(k_enc, pair, klen);
            k_enc[klen] = '\0';
            strncpy(v_enc, eq + 1, sizeof(v_enc) - 1);
        } else {
            strncpy(k_enc, pair, sizeof(k_enc) - 1);
            v_enc[0] = '\0';
        }

        char k[65] = {0};
        char v[1025] = {0};
        if (url_decode(k_enc, k, sizeof(k)) && url_decode(v_enc, v, sizeof(v))) {
            strncpy(form->items[form->count].key, k, sizeof(form->items[form->count].key) - 1);
            strncpy(form->items[form->count].value, v, sizeof(form->items[form->count].value) - 1);
            form->count++;
        }

        if (!amp) break;
        p = amp + 1;
    }
}

static const char* form_get(Form* f, const char* key) {
    for (size_t i = 0; i < f->count; ++i) {
        if (strcmp(f->items[i].key, key) == 0) return f->items[i].value;
    }
    return NULL;
}

static void trim(char* s) {
    size_t len = strlen(s);
    size_t i = 0;
    while (i < len && (s[i] == ' ' || s[i] == '\t')) i++;
    if (i > 0) memmove(s, s + i, len - i + 1);
    len = strlen(s);
    while (len > 0 && (s[len - 1] == ' ' || s[len - 1] == '\t' || s[len - 1] == '\r' || s[len - 1] == '\n')) {
        s[--len] = '\0';
    }
}

static const char* get_header(const char* headers, const char* name, char* out, size_t out_sz) {
    size_t name_len = strlen(name);
    const char* p = headers;
    while (*p) {
        const char* nl = strstr(p, "\r\n");
        size_t len = nl ? (size_t)(nl - p) : strlen(p);
        if (len == 0) break;
        char line[2048];
        if (len >= sizeof(line)) len = sizeof(line) - 1;
        memcpy(line, p, len);
        line[len] = '\0';
        char* colon = strchr(line, ':');
        if (colon) {
            *colon = '\0';
            trim(line);
            for (char* q = line; *q; ++q) if (*q >= 'A' && *q <= 'Z') *q = (char)(*q - 'A' + 'a');
            char lname[256];
            strncpy(lname, line, sizeof(lname) - 1); lname[sizeof(lname) - 1] = '\0';
            if (strcmp(lname, name) == 0) {
                char* val = colon + 1;
                trim(val);
                strncpy(out, val, out_sz - 1);
                out[out_sz - 1] = '\0';
                return out;
            }
        }
        if (!nl) break;
        p = nl + 2;
    }
    return NULL;
}

static void parse_cookies(const char* cookie, char* sid_out, size_t sid_out_sz) {
    sid_out[0] = '\0';
    const char* p = cookie;
    while (*p) {
        while (*p == ' ' || *p == '\t' || *p == ';') p++;
        const char* eq = strchr(p, '=');
        if (!eq) break;
        size_t keylen = (size_t)(eq - p);
        const char* semi = strchr(eq + 1, ';');
        size_t vallen = semi ? (size_t)(semi - (eq + 1)) : strlen(eq + 1);
        if (keylen == 3 && strncmp(p, "SID", 3) == 0) {
            size_t n = vallen < sid_out_sz - 1 ? vallen : sid_out_sz - 1;
            memcpy(sid_out, eq + 1, n);
            sid_out[n] = '\0';
            return;
        }
        p = semi ? semi + 1 : eq + 1 + vallen;
    }
}

static Session* find_session_by_id(const char* sid) {
    if (!sid || !*sid) return NULL;
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (g_sessions[i].in_use && strncmp(g_sessions[i].id, sid, 32) == 0) {
            return &g_sessions[i];
        }
    }
    return NULL;
}

static Session* create_session(char* set_cookie_header, size_t set_cookie_sz) {
    for (int i = 0; i < MAX_SESSIONS; ++i) {
        if (!g_sessions[i].in_use) {
            g_sessions[i].in_use = 1;
            generate_token_hex(16, g_sessions[i].id, sizeof(g_sessions[i].id));
            generate_token_hex(32, g_sessions[i].csrf, sizeof(g_sessions[i].csrf));
            snprintf(set_cookie_header, set_cookie_sz, "Set-Cookie: SID=%s; Path=/; HttpOnly; SameSite=Strict; Secure\r\n", g_sessions[i].id);
            return &g_sessions[i];
        }
    }
    set_cookie_header[0] = '\0';
    return NULL;
}

static Session* get_or_create_session(const char* headers, char* set_cookie_header, size_t set_cookie_sz) {
    char cookie[2048];
    cookie[0] = '\0';
    if (get_header(headers, "cookie", cookie, sizeof(cookie))) {
        char sid[128];
        parse_cookies(cookie, sid, sizeof(sid));
        Session* s = find_session_by_id(sid);
        if (s) {
            set_cookie_header[0] = '\0';
            return s;
        }
    }
    return create_session(set_cookie_header, set_cookie_sz);
}

static bool valid_display_name(const char* s) {
    size_t len = strlen(s);
    if (len == 0 || len > 60) return false;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c < 32 || c == 127) return false;
    }
    return true;
}

static bool valid_email(const char* s) {
    size_t len = strlen(s);
    if (len < 3 || len > 120) return false;
    if (!strchr(s, '@')) return false;
    if (strchr(s, ' ')) return false;
    return true;
}

static void send_all(int fd, const char* data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, data + sent, len - sent, 0);
        if (n <= 0) {
            if (errno == EINTR) continue;
            break;
        }
        sent += (size_t)n;
    }
}

static void build_form_page(const Session* sess, const char* message, char** out_html, size_t* out_len) {
    char msg_esc[512];
    msg_esc[0] = '\0';
    if (message && *message) {
        html_escape(message, msg_esc, sizeof(msg_esc));
    }
    const char* head = "<!doctype html><html><head><meta charset=\"utf-8\">"
                       "<meta http-equiv=\"Content-Security-Policy\" content=\"default-src 'none'; style-src 'self' 'unsafe-inline'; form-action 'self'; base-uri 'none'\">"
                       "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">"
                       "<title>User Settings</title></head><body><h1>User Settings</h1>";
    const char* form1 = "<form method=\"POST\" action=\"/update\">"
                        "<label>Display Name: <input type=\"text\" name=\"displayName\" maxlength=\"60\" required></label><br>"
                        "<label>Email: <input type=\"email\" name=\"email\" maxlength=\"120\" required></label><br>";
    const char* form2 = "<button type=\"submit\">Update</button></form></body></html>";
    size_t need = strlen(head) + strlen(form1) + strlen(form2) + strlen(sess->csrf) + 256 + strlen(msg_esc);
    char* html = (char*)malloc(need);
    if (!html) { *out_html = NULL; *out_len = 0; return; }
    if (msg_esc[0]) {
        snprintf(html, need, "%s<p>%s</p>%s<input type=\"hidden\" name=\"csrf\" value=\"%s\">%s",
                 head, msg_esc, form1, sess->csrf, form2);
    } else {
        snprintf(html, need, "%s%s<input type=\"hidden\" name=\"csrf\" value=\"%s\">%s",
                 head, form1, sess->csrf, form2);
    }
    *out_len = strlen(html);
    *out_html = html;
}

static void respond_simple(int cfd, int code, const char* text) {
    char body[512];
    snprintf(body, sizeof(body), "<h1>%d %s</h1>", code, text);
    char hdr[1024];
    int n = snprintf(hdr, sizeof(hdr),
                     "HTTP/1.1 %d %s\r\n"
                     "Content-Type: text/html; charset=utf-8\r\n"
                     "X-Content-Type-Options: nosniff\r\n"
                     "X-Frame-Options: DENY\r\n"
                     "Referrer-Policy: no-referrer\r\n"
                     "Content-Length: %zu\r\n\r\n", code, text, strlen(body));
    send_all(cfd, hdr, (size_t)n);
    send_all(cfd, body, strlen(body));
}

static void handle_client(int cfd) {
    char req[MAX_REQ + 1];
    size_t total = 0;
    ssize_t n;
    size_t header_end = 0;
    size_t content_length = 0;
    int headers_parsed = 0;
    while (total < MAX_REQ) {
        n = recv(cfd, req + total, MAX_REQ - total, 0);
        if (n <= 0) break;
        total += (size_t)n;
        req[total] = '\0';
        if (!headers_parsed) {
            char* pos = strstr(req, "\r\n\r\n");
            if (pos) {
                header_end = (size_t)(pos - req) + 4;
                headers_parsed = 1;
                // find content-length
                char headers_copy[4096];
                size_t hl = header_end > sizeof(headers_copy) - 1 ? sizeof(headers_copy) - 1 : header_end;
                memcpy(headers_copy, req, hl);
                headers_copy[hl] = '\0';
                char clval[128];
                if (get_header(headers_copy, "content-length", clval, sizeof(clval))) {
                    long cl = strtol(clval, NULL, 10);
                    if (cl > 0 && cl < (long)MAX_BODY) content_length = (size_t)cl;
                }
                if (total >= header_end + content_length) break;
            }
        } else {
            if (total >= header_end + content_length) break;
        }
    }
    if (total == 0) { close(cfd); return; }

    // parse request line
    char* first_crlf = strstr(req, "\r\n");
    if (!first_crlf) { respond_simple(cfd, 400, "Bad Request"); close(cfd); return; }
    char reqline[512];
    size_t rl = (size_t)(first_crlf - req);
    if (rl >= sizeof(reqline)) rl = sizeof(reqline) - 1;
    memcpy(reqline, req, rl);
    reqline[rl] = '\0';
    char method[8] = {0}, path[256] = {0}, version[16] = {0};
    sscanf(reqline, "%7s %255s %15s", method, path, version);

    // headers segment
    size_t headers_len = header_end >= 2 ? header_end - 2 : header_end;
    char headers_seg[4096];
    size_t hl = headers_len > sizeof(headers_seg) - 1 ? sizeof(headers_seg) - 1 : headers_len;
    memcpy(headers_seg, req, hl);
    headers_seg[hl] = '\0';

    // body
    char* body = NULL;
    size_t body_len = 0;
    if (header_end < total) {
        body = req + header_end;
        body_len = total - header_end;
    }

    char set_cookie[256];
    Session* sess = get_or_create_session(headers_seg, set_cookie, sizeof(set_cookie));
    if (!sess) { respond_simple(cfd, 503, "Service Unavailable"); close(cfd); return; }

    if (strcmp(method, "GET") == 0 && strcmp(path, "/") == 0) {
        char* html = NULL; size_t html_len = 0;
        build_form_page(sess, "", &html, &html_len);
        if (!html) { respond_simple(cfd, 500, "Internal Server Error"); close(cfd); return; }
        char hdr[1024];
        int nlen = snprintf(hdr, sizeof(hdr),
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/html; charset=utf-8\r\n"
                            "X-Content-Type-Options: nosniff\r\n"
                            "X-Frame-Options: DENY\r\n"
                            "Referrer-Policy: no-referrer\r\n"
                            "%s"
                            "Content-Length: %zu\r\n\r\n", set_cookie, html_len);
        send_all(cfd, hdr, (size_t)nlen);
        send_all(cfd, html, html_len);
        free(html);
    } else if (strcmp(method, "POST") == 0 && strcmp(path, "/update") == 0) {
        // check content-type
        char ctype[256];
        ctype[0] = '\0';
        int is_form = 0;
        if (get_header(headers_seg, "content-type", ctype, sizeof(ctype))) {
            for (char* p = ctype; *p; ++p) if (*p >= 'A' && *p <= 'Z') *p = (char)(*p - 'A' + 'a');
            if (strstr(ctype, "application/x-www-form-urlencoded")) is_form = 1;
        }
        if (!is_form) {
            respond_simple(cfd, 415, "Unsupported Media Type");
        } else {
            char body_copy[MAX_BODY + 1];
            size_t bl = body_len > MAX_BODY ? MAX_BODY : body_len;
            memcpy(body_copy, body, bl);
            body_copy[bl] = '\0';
            Form form;
            parse_form_urlencoded(body_copy, &form);
            const char* csrf = form_get(&form, "csrf");
            if (!csrf || !const_time_eq(csrf, sess->csrf)) {
                respond_simple(cfd, 403, "Forbidden");
            } else {
                const char* dn = form_get(&form, "displayName");
                const char* em = form_get(&form, "email");
                if (!dn) dn = "";
                if (!em) em = "";
                if (!valid_display_name(dn) || !valid_email(em)) {
                    char* html = NULL; size_t html_len = 0;
                    build_form_page(sess, "Invalid input.", &html, &html_len);
                    if (!html) { respond_simple(cfd, 500, "Internal Server Error"); }
                    else {
                        char hdr[1024];
                        int nlen = snprintf(hdr, sizeof(hdr),
                                            "HTTP/1.1 400 Bad Request\r\n"
                                            "Content-Type: text/html; charset=utf-8\r\n"
                                            "X-Content-Type-Options: nosniff\r\n"
                                            "X-Frame-Options: DENY\r\n"
                                            "Referrer-Policy: no-referrer\r\n"
                                            "%s"
                                            "Content-Length: %zu\r\n\r\n", set_cookie, html_len);
                        send_all(cfd, hdr, (size_t)nlen);
                        send_all(cfd, html, html_len);
                        free(html);
                    }
                } else {
                    // rotate CSRF
                    generate_token_hex(32, sess->csrf, sizeof(sess->csrf));
                    char msg[256];
                    snprintf(msg, sizeof(msg), "Settings updated for %s (%s)", dn, em);
                    char* html = NULL; size_t html_len = 0;
                    build_form_page(sess, msg, &html, &html_len);
                    if (!html) { respond_simple(cfd, 500, "Internal Server Error"); }
                    else {
                        char hdr[1024];
                        int nlen = snprintf(hdr, sizeof(hdr),
                                            "HTTP/1.1 200 OK\r\n"
                                            "Content-Type: text/html; charset=utf-8\r\n"
                                            "X-Content-Type-Options: nosniff\r\n"
                                            "X-Frame-Options: DENY\r\n"
                                            "Referrer-Policy: no-referrer\r\n"
                                            "%s"
                                            "Content-Length: %zu\r\n\r\n", set_cookie, html_len);
                        send_all(cfd, hdr, (size_t)nlen);
                        send_all(cfd, html, html_len);
                        free(html);
                    }
                }
            }
        }
    } else {
        respond_simple(cfd, 404, "Not Found");
    }
    close(cfd);
}

static int run_server(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) return 1;
    int yes = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) { close(fd); return 2; }
    if (listen(fd, 16) < 0) { close(fd); return 3; }
    for (;;) {
        struct sockaddr_in caddr;
        socklen_t clen = sizeof(caddr);
        int cfd = accept(fd, (struct sockaddr*)&caddr, &clen);
        if (cfd < 0) {
            if (errno == EINTR) continue;
            break;
        }
        handle_client(cfd);
    }
    close(fd);
    return 0;
}

/* 5 Test cases */
static void assert_test(int cond, const char* name, int* pass, int* fail) {
    if (cond) { printf("[OK] %s\n", name); (*pass)++; }
    else { printf("[FAIL] %s\n", name); (*fail)++; }
}

int main(void) {
    int passed = 0, failed = 0;

    // Test 1: Token uniqueness and size
    char t1[33], t2[33];
    generate_token_hex(16, t1, sizeof(t1));
    generate_token_hex(16, t2, sizeof(t2));
    assert_test(strcmp(t1, t2) != 0 && strlen(t1) == 32 && strlen(t2) == 32, "Token uniqueness and size", &passed, &failed);

    // Test 2: Constant time equals
    assert_test(const_time_eq("abc", "abc") && !const_time_eq("abc", "abd"), "Constant time equals", &passed, &failed);

    // Test 3: URL decode
    char dec[64];
    assert_test(url_decode("a%2Bb%20c", dec, sizeof(dec)) && strcmp(dec, "a+b c") == 0, "URL decode", &passed, &failed);

    // Test 4: HTML escape
    char esc[64];
    html_escape("<a & b>", esc, sizeof(esc));
    assert_test(strcmp(esc, "&lt;a &amp; b&gt;") == 0, "HTML escape", &passed, &failed);

    // Test 5: Form contains CSRF field
    Session s; memset(&s, 0, sizeof(s)); s.in_use = 1;
    generate_token_hex(32, s.csrf, sizeof(s.csrf));
    char* html = NULL; size_t html_len = 0;
    build_form_page(&s, "", &html, &html_len);
    int contains = 0;
    if (html) {
        contains = strstr(html, "name=\"csrf\"") != NULL;
        free(html);
    }
    assert_test(contains, "Form contains CSRF field", &passed, &failed);

    printf("Passed: %d Failed: %d\n", passed, failed);

    const char* run = getenv("RUN_SERVER");
    if (run && strcmp(run, "1") == 0) {
        return run_server(8080);
    }
    return failed == 0 ? 0 : 1;
}