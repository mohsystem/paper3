#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_LEN 1024u
#define MAX_BODY_LEN 4096u
#define MAX_HTML_LEN 65536u

typedef struct {
    int ok;
    char *s;      // allocated string
} Decoded;

static int is_ascii_printable_or_space(unsigned char c) {
    return c == 9 || c == 10 || c == 13 || (c >= 32 && c <= 126);
}

static int validate_input(const char *s) {
    if (s == NULL) return 0;
    size_t n = strlen(s);
    if (n > MAX_INPUT_LEN) return 0;
    for (size_t i = 0; i < n; ++i) {
        if (!is_ascii_printable_or_space((unsigned char)s[i])) return 0;
    }
    return 1;
}

static char* html_escape(const char *in) {
    if (in == NULL) return NULL;
    size_t n = strlen(in);
    size_t cap = n + n / 8 + 32;
    if (cap > MAX_HTML_LEN) cap = MAX_HTML_LEN;
    char *out = (char*)malloc(cap + 1);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        const char *rep = NULL;
        char c = in[i];
        switch (c) {
            case '&': rep = "&amp;"; break;
            case '<': rep = "&lt;"; break;
            case '>': rep = "&gt;"; break;
            case '"': rep = "&quot;"; break;
            case '\'': rep = "&#39;"; break;
            default: rep = NULL; break;
        }
        if (rep) {
            size_t rl = strlen(rep);
            if (j + rl > MAX_HTML_LEN) { j = MAX_HTML_LEN; break; }
            memcpy(out + j, rep, rl);
            j += rl;
        } else {
            if (j + 1 > MAX_HTML_LEN) { j = MAX_HTML_LEN; break; }
            out[j++] = c;
        }
    }
    out[j] = '\0';
    return out;
}

static int hexval(unsigned char c) {
    if (c >= '0' && c <= '9') return (int)(c - '0');
    if (c >= 'a' && c <= 'f') return 10 + (int)(c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (int)(c - 'A');
    return -1;
}

static Decoded url_decode(const char *in) {
    Decoded res; res.ok = 0; res.s = NULL;
    if (!in) return res;
    size_t n = strlen(in);
    char *out = (char*)malloc(n + 1);
    if (!out) return res;
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)in[i];
        if (c == '+') {
            out[j++] = ' ';
        } else if (c == '%') {
            if (i + 2 >= n) { free(out); return res; }
            int hi = hexval((unsigned char)in[i + 1]);
            int lo = hexval((unsigned char)in[i + 2]);
            if (hi < 0 || lo < 0) { free(out); return res; }
            out[j++] = (char)((hi << 4) | lo);
            i += 2;
        } else {
            out[j++] = (char)c;
        }
        if (j > MAX_INPUT_LEN) { free(out); return res; }
    }
    out[j] = '\0';
    res.ok = 1;
    res.s = out;
    return res;
}

static char* str_lower_trim(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s);
    size_t b = 0, e = n;
    while (b < e && isspace((unsigned char)s[b])) b++;
    while (e > b && isspace((unsigned char)s[e - 1])) e--;
    size_t m = e - b;
    char *t = (char*)malloc(m + 1);
    if (!t) return NULL;
    for (size_t i = 0; i < m; ++i) {
        t[i] = (char)tolower((unsigned char)s[b + i]);
    }
    t[m] = '\0';
    return t;
}

static int content_type_is_form_urlencoded(const char *ctype) {
    if (!ctype) return 0;
    const char *sc = strchr(ctype, ';');
    size_t len = sc ? (size_t)(sc - ctype) : strlen(ctype);
    char *head = (char*)malloc(len + 1);
    if (!head) return 0;
    memcpy(head, ctype, len);
    head[len] = '\0';
    char *norm = str_lower_trim(head);
    free(head);
    if (!norm) return 0;
    int ok = strcmp(norm, "application/x-www-form-urlencoded") == 0;
    free(norm);
    return ok;
}

static char* build_html_page(const char *maybeMessage, const char *maybeError) {
    // Build HTML string with dynamic buffer
    size_t cap = 4096;
    if (cap > MAX_HTML_LEN) cap = MAX_HTML_LEN;
    char *buf = (char*)malloc(cap);
    if (!buf) return NULL;
    size_t len = 0;

    // Helper macro for safe append
    #define APPEND_FMT(fmt, ...) \
        do { \
            int need = snprintf(NULL, 0, fmt, __VA_ARGS__); \
            if (need < 0) { free(buf); return NULL; } \
            size_t need_sz = (size_t)need; \
            if (len + need_sz + 1 > cap) { \
                size_t newcap = cap; \
                while (len + need_sz + 1 > newcap) { newcap *= 2; if (newcap > MAX_HTML_LEN) { newcap = MAX_HTML_LEN; break; } } \
                if (len + need_sz + 1 > newcap) { /* cannot fit */ need_sz = (newcap > len) ? (newcap - len - 1) : 0; } \
                char *tmp = (char*)realloc(buf, newcap); \
                if (!tmp) { free(buf); return NULL; } \
                buf = tmp; cap = newcap; \
            } \
            int wrote = snprintf(buf + len, cap - len, fmt, __VA_ARGS__); \
            if (wrote < 0) { free(buf); return NULL; } \
            if ((size_t)wrote > cap - len - 1) wrote = (int)(cap - len - 1); \
            len += (size_t)wrote; \
        } while (0)

    APPEND_FMT("%s", "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">");
    APPEND_FMT("%s", "<meta http-equiv=\"X-Content-Type-Options\" content=\"nosniff\">");
    APPEND_FMT("%s", "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    APPEND_FMT("%s", "<title>Echo Form</title>");
    APPEND_FMT("%s", "<style>body{font-family:sans-serif;margin:2rem;} .box{padding:1rem;border:1px solid #ccc;border-radius:8px;max-width:640px;} .err{color:#b00020;} .msg{background:#f6f8fa;padding:.75rem;border-radius:6px;white-space:pre-wrap;}</style>");
    APPEND_FMT("%s", "</head><body><div class=\"box\"><h1>Echo your message</h1>");

    if (maybeError && maybeError[0]) {
        char *esc = html_escape(maybeError);
        if (!esc) { free(buf); return NULL; }
        APPEND_FMT("<p class=\"err\">Error: %s</p>", esc);
        free(esc);
    }

    APPEND_FMT("%s", "<form method=\"POST\" action=\"\">");
    APPEND_FMT("%s", "<label for=\"message\">Enter text (max 1024 ASCII chars):</label><br>");
    APPEND_FMT("%s", "<textarea id=\"message\" name=\"message\" rows=\"5\" cols=\"60\" maxlength=\"1024\" required></textarea><br>");
    APPEND_FMT("%s", "<button type=\"submit\">Submit</button></form>");

    if (maybeMessage) {
        char *escm = html_escape(maybeMessage);
        if (!escm) { free(buf); return NULL; }
        APPEND_FMT("%s", "<h2>Echoed message</h2><div class=\"msg\">");
        APPEND_FMT("%s", escm);
        APPEND_FMT("%s", "</div>");
        free(escm);
    }

    APPEND_FMT("%s", "</div></body></html>");

    #undef APPEND_FMT
    return buf;
}

static char* get_form_field_message(const char *body) {
    if (!body) return NULL;
    size_t n = strlen(body);
    size_t start = 0;
    unsigned int pairCount = 0;
    while (start <= n) {
        if (++pairCount > 200u) break;
        size_t amp = start;
        while (amp < n && body[amp] != '&') amp++;
        size_t seglen = amp - start;
        const char *seg = body + start;
        // Find '='
        size_t eqpos = 0;
        int hasEq = 0;
        for (size_t i = 0; i < seglen; ++i) {
            if (seg[i] == '=') { eqpos = i; hasEq = 1; break; }
        }
        size_t klen = hasEq ? eqpos : seglen;
        size_t vlen = hasEq ? (seglen - eqpos - 1) : 0;

        char *kenc = (char*)malloc(klen + 1);
        if (!kenc) return NULL;
        memcpy(kenc, seg, klen); kenc[klen] = '\0';

        char *venc = (char*)malloc(vlen + 1);
        if (!venc) { free(kenc); return NULL; }
        if (hasEq && vlen > 0) memcpy(venc, seg + eqpos + 1, vlen);
        venc[vlen] = '\0';

        Decoded kd = url_decode(kenc);
        Decoded vd = url_decode(venc);
        free(kenc); free(venc);

        if (kd.ok && vd.ok) {
            int isMsg = (strcmp(kd.s, "message") == 0);
            free(kd.s);
            if (isMsg) {
                // Limit length
                if (strlen(vd.s) > MAX_INPUT_LEN) {
                    free(vd.s);
                    return NULL;
                }
                return vd.s; // caller must free
            } else {
                free(vd.s);
            }
        } else {
            if (kd.ok) free(kd.s);
            if (vd.ok) free(vd.s);
        }

        if (amp >= n) break;
        start = amp + 1;
    }
    return NULL;
}

static char* handle_request(const char *method, const char *contentType, const char *bodyRaw) {
    if (!method) method = "";
    char *lower = NULL;
    {
        size_t mlen = strlen(method);
        lower = (char*)malloc(mlen + 1);
        if (!lower) return NULL;
        for (size_t i = 0; i < mlen; ++i) lower[i] = (char)tolower((unsigned char)method[i]);
        lower[mlen] = '\0';
    }
    int is_post = strcmp(lower, "post") == 0;
    free(lower);

    if (!is_post) {
        return build_html_page(NULL, NULL);
    }
    if (!content_type_is_form_urlencoded(contentType)) {
        return build_html_page(NULL, "Unsupported Content-Type. Use application/x-www-form-urlencoded.");
    }
    if (bodyRaw && strlen(bodyRaw) > MAX_BODY_LEN) {
        return build_html_page(NULL, "Request body too large.");
    }
    char *msg = get_form_field_message(bodyRaw ? bodyRaw : "");
    if (!msg) {
        return build_html_page(NULL, "Missing 'message' field.");
    }
    if (!validate_input(msg)) {
        free(msg);
        return build_html_page(NULL, "Invalid input. Use ASCII printable characters up to 1024 length.");
    }
    char *html = build_html_page(msg, NULL);
    free(msg);
    return html;
}

static int is_cgi_environment(void) {
    const char *rm = getenv("REQUEST_METHOD");
    const char *gi = getenv("GATEWAY_INTERFACE");
    return (rm != NULL) || (gi != NULL);
}

static char* read_stdin_body(size_t contentLength) {
    if (contentLength > MAX_BODY_LEN) contentLength = MAX_BODY_LEN;
    char *buf = (char*)malloc(contentLength + 1);
    if (!buf) return NULL;
    size_t total = 0;
    while (total < contentLength) {
        size_t toRead = contentLength - total;
        if (toRead > 4096u) toRead = 4096u;
        size_t got = fread(buf + total, 1, toRead, stdin);
        if (got == 0) break;
        total += got;
    }
    buf[total] = '\0';
    return buf;
}

static void run_tests(void) {
    struct Case { const char *name; const char *method; const char *ctype; const char *body; } cases[5] = {
        {"GET shows form", "GET", "", ""},
        {"POST ok simple", "POST", "application/x-www-form-urlencoded", "message=Hello+World"},
        {"POST escapes HTML", "POST", "application/x-www-form-urlencoded", "message=%3Cb%3Ebold%3C%2Fb%3E+%26+%22q%22"},
        {"POST too long", "POST", "application/x-www-form-urlencoded", NULL}, // constructed below
        {"POST invalid encoding", "POST", "application/x-www-form-urlencoded", "message=Hello%ZZWorld"}
    };
    // Build long body for case 4
    size_t longLen = 7 + 1100; // "message=" + 1100 'A'
    char *longBody = (char*)malloc(longLen + 1);
    if (longBody) {
        strcpy(longBody, "message=");
        memset(longBody + 8, 'A', 1100);
        longBody[longLen] = '\0';
        cases[3].body = longBody;
    }

    for (int i = 0; i < 5; ++i) {
        char *html = handle_request(cases[i].method, cases[i].ctype, cases[i].body);
        if (!html) {
            printf("==== %s ====\nError generating HTML\n", cases[i].name);
            continue;
        }
        size_t hlen = strlen(html);
        printf("==== %s ====\nHTML length: %zu\n", cases[i].name, hlen);
        // Print small snippet: look for msg or err
        const char *msg = strstr(html, "<div class=\"msg\">");
        if (msg) {
            size_t off = (size_t)(msg - html);
            size_t sn = (hlen - off) > 200 ? 200 : (hlen - off);
            fwrite(msg, 1, sn, stdout);
            printf("\n");
        } else {
            const char *err = strstr(html, "<p class=\"err\">");
            if (err) {
                size_t off = (size_t)(err - html);
                size_t sn = (hlen - off) > 200 ? 200 : (hlen - off);
                fwrite(err, 1, sn, stdout);
                printf("\n");
            } else {
                size_t sn = hlen > 200 ? 200 : hlen;
                fwrite(html, 1, sn, stdout);
                printf("\n");
            }
        }
        free(html);
    }
    if (longBody) free(longBody);
}

int main(void) {
    if (is_cgi_environment()) {
        const char *rm = getenv("REQUEST_METHOD");
        const char *ct = getenv("CONTENT_TYPE");
        const char *cl = getenv("CONTENT_LENGTH");
        size_t clen = 0;
        if (cl) {
            size_t i = 0;
            int ok = 1;
            for (; cl[i] != '\0'; ++i) if (!isdigit((unsigned char)cl[i])) { ok = 0; break; }
            if (ok && i > 0) {
                unsigned long v = strtoul(cl, NULL, 10);
                if (v > MAX_BODY_LEN) v = MAX_BODY_LEN;
                clen = (size_t)v;
            }
        }
        char *body = read_stdin_body(clen);
        char *html = handle_request(rm ? rm : "", ct ? ct : "", body ? body : "");
        if (body) free(body);
        if (!html) {
            const char *err = "<html><body><p>Internal Server Error</p></body></html>";
            printf("Content-Type: text/html; charset=UTF-8\r\n");
            printf("X-Content-Type-Options: nosniff\r\n");
            printf("Content-Length: %zu\r\n\r\n", strlen(err));
            printf("%s", err);
            return 0;
        }
        printf("Content-Type: text/html; charset=UTF-8\r\n");
        printf("X-Content-Type-Options: nosniff\r\n");
        printf("Content-Length: %zu\r\n\r\n", strlen(html));
        printf("%s", html);
        free(html);
        return 0;
    } else {
        run_tests();
        return 0;
    }
}