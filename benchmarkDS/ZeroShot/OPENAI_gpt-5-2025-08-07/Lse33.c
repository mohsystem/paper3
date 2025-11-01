#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int ok;         // 1 if valid redirect, 0 otherwise
    char* target;   // normalized/accepted URL when ok=1
    char* error;    // error message when ok=0
} RedirectResult;

static char* str_duplicate(const char* s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char* p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static int starts_with_ci(const char* s, const char* pfx) {
    size_t i=0;
    for (; pfx[i] && s[i]; ++i) {
        if (tolower((unsigned char)s[i]) != tolower((unsigned char)pfx[i])) return 0;
    }
    return pfx[i] == '\0';
}

static char* to_lower_dup(const char* s, size_t len) {
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) out[i] = (char)tolower((unsigned char)s[i]);
    out[len] = '\0';
    return out;
}

static char hexval(char c) {
    if (c >= '0' && c <= '9') return (char)(c - '0');
    c = (char)tolower((unsigned char)c);
    if (c >= 'a' && c <= 'f') return (char)(10 + c - 'a');
    return (char)0;
}

static char* url_decode(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '%' && i + 2 < len && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])) {
            char hi = hexval(s[i+1]);
            char lo = hexval(s[i+2]);
            out[oi++] = (char)((hi << 4) | lo);
            i += 2;
        } else if (s[i] == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = s[i];
        }
    }
    out[oi] = '\0';
    return out;
}

RedirectResult redirect_invalid_url(const char* rawUrl) {
    RedirectResult res = {0, NULL, NULL};
    if (rawUrl == NULL) { res.error = str_duplicate("Missing URL parameter."); return res; }
    size_t L = strlen(rawUrl);
    if (L > 2048) { res.error = str_duplicate("URL too long."); return res; }
    for (size_t i = 0; i < L; ++i) {
        if (rawUrl[i] == '\r' || rawUrl[i] == '\n') {
            res.error = str_duplicate("Invalid characters in URL.");
            return res;
        }
    }

    const char* s = rawUrl;
    size_t pos = 0;
    if (starts_with_ci(s, "http://")) {
        pos = 7;
    } else if (starts_with_ci(s, "https://")) {
        pos = 8;
    } else {
        res.error = str_duplicate("URL must include http or https scheme.");
        return res;
    }

    // Find end of authority
    size_t len = L;
    size_t host_start = pos;
    size_t scan_end = len;
    for (size_t i = host_start; i < len; ++i) {
        if (s[i] == '/' || s[i] == '?' || s[i] == '#') { scan_end = i; break; }
    }

    // Reject userinfo if '@' before scan_end
    for (size_t i = host_start; i < scan_end; ++i) {
        if (s[i] == '@') {
            res.error = str_duplicate("Userinfo not allowed in URL.");
            return res;
        }
    }

    // Extract host (up to ':' or scan_end)
    size_t host_end = scan_end;
    for (size_t i = host_start; i < scan_end; ++i) {
        if (s[i] == ':') { host_end = i; break; }
    }
    if (host_end <= host_start) { res.error = str_duplicate("Invalid host."); return res; }

    char* host_lc = to_lower_dup(s + host_start, host_end - host_start);
    if (!host_lc) { res.error = str_duplicate("Memory error."); return res; }
    int host_ok = (strcmp(host_lc, "example.com") == 0);
    free(host_lc);
    if (!host_ok) { res.error = str_duplicate("URL must be on example.com."); return res; }

    // All good: echo original URL back (CRLF already checked)
    res.ok = 1;
    res.target = str_duplicate(rawUrl);
    return res;
}

static void cgi_respond() {
    const char* qs = getenv("QUERY_STRING");
    const char* method = getenv("REQUEST_METHOD");
    (void)method;

    char* urlParam = NULL;
    if (qs && *qs) {
        // Parse key-value pairs
        const char* p = qs;
        while (*p) {
            const char* key_start = p;
            const char* eq = strchr(p, '=');
            const char* amp = strchr(p, '&');
            if (!amp) amp = p + strlen(p);
            if (eq && eq < amp) {
                size_t key_len = (size_t)(eq - key_start);
                char* key = (char*)malloc(key_len + 1);
                if (!key) break;
                memcpy(key, key_start, key_len);
                key[key_len] = '\0';
                size_t val_len = (size_t)(amp - (eq + 1));
                char* val_enc = (char*)malloc(val_len + 1);
                if (!val_enc) { free(key); break; }
                memcpy(val_enc, eq + 1, val_len);
                val_enc[val_len] = '\0';

                if (strcmp(key, "url") == 0) {
                    char* decoded = url_decode(val_enc);
                    if (decoded) {
                        urlParam = decoded; // take ownership
                    }
                }
                free(key);
                free(val_enc);
            }
            if (*amp == '\0') break;
            p = amp + 1;
        }
    }

    RedirectResult r = redirect_invalid_url(urlParam);
    if (urlParam) free(urlParam);

    if (r.ok) {
        printf("Status: 302 Found\r\n");
        printf("Location: %s\r\n", r.target);
        printf("Cache-Control: no-store\r\n");
        printf("\r\n");
    } else {
        const char* err = r.error ? r.error : "Invalid request";
        size_t blen = strlen("Error: ") + strlen(err);
        printf("Status: 400 Bad Request\r\n");
        printf("Content-Type: text/plain; charset=utf-8\r\n");
        printf("Content-Length: %zu\r\n", blen);
        printf("\r\n");
        printf("Error: %s", err);
    }

    if (r.target) free(r.target);
    if (r.error) free(r.error);
}

int main(void) {
    // If running as CGI (REQUEST_METHOD set), handle request; else run tests
    if (getenv("REQUEST_METHOD")) {
        cgi_respond();
        return 0;
    }

    const char* tests[] = {
        "https://example.com/",
        "http://example.com/path?x=1",
        "https://sub.example.com/",
        "https://example.com.evil.com/",
        "https://example.com:8080/path#frag"
    };
    for (size_t i = 0; i < 5; ++i) {
        RedirectResult r = redirect_invalid_url(tests[i]);
        if (r.ok) {
            printf("OK -> %s\n", r.target);
        } else {
            printf("ERR -> %s\n", r.error ? r.error : "Unknown error");
        }
        if (r.target) free(r.target);
        if (r.error) free(r.error);
    }
    return 0;
}