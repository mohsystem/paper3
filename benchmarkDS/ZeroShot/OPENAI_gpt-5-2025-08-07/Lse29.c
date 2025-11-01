#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_URL_LENGTH 2048

static char* str_dup(const char* s) {
    size_t n = strlen(s) + 1;
    char* p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

static char* str_trim(const char* s) {
    if (!s) return str_dup("");
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = s + strlen(s);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

static int istarts_with(const char* s, const char* pfx) {
    size_t i = 0;
    while (pfx[i]) {
        char a = s[i], b = pfx[i];
        if (a == '\0') return 0;
        if (a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
        if (b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');
        if (a != b) return 0;
        i++;
    }
    return 1;
}

static int host_allowed(const char* host) {
    // Allow exact "example.com" or any subdomain "*.example.com"
    size_t len = strlen(host);
    const char* needle = "example.com";
    size_t nlen = strlen(needle);
    if (len < nlen) return 0;
    const char* tail = host + (len - nlen);
    if (strcasecmp(tail, needle) != 0) return 0;
    if (len == nlen) return 1; // exact match
    // Ensure preceding char is a dot
    return (*(tail - 1) == '.') ? 1 : 0;
}

char* safe_redirect(const char* target) {
    if (target == NULL) {
        return str_dup("/");
    }

    char* t = str_trim(target);
    if (!t) return str_dup("/");

    size_t tlen = strlen(t);
    if (tlen == 0 || tlen > MAX_URL_LENGTH) {
        free(t);
        return str_dup("/");
    }

    for (size_t i = 0; i < tlen; ++i) {
        if (t[i] == '\r' || t[i] == '\n') {
            free(t);
            return str_dup("/");
        }
    }

    if (!(istarts_with(t, "http://") || istarts_with(t, "https://"))) {
        free(t);
        return str_dup("/");
    }

    const char* scheme_end = strstr(t, "://");
    if (!scheme_end) {
        free(t);
        return str_dup("/");
    }
    const char* host_start = scheme_end + 3;
    if (*host_start == '\0') {
        free(t);
        return str_dup("/");
    }

    const char* host_end = host_start;
    while (*host_end && *host_end != '/' && *host_end != '?' && *host_end != '#' && *host_end != ':') {
        host_end++;
    }
    if (host_end == host_start) {
        free(t);
        return str_dup("/");
    }

    size_t hlen = (size_t)(host_end - host_start);
    if (hlen == 0 || hlen > 253) {
        free(t);
        return str_dup("/");
    }

    char* host = (char*)malloc(hlen + 1);
    if (!host) {
        free(t);
        return str_dup("/");
    }
    for (size_t i = 0; i < hlen; ++i) {
        char c = host_start[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '.')) {
            free(host);
            free(t);
            return str_dup("/");
        }
        host[i] = (char)tolower((unsigned char)c);
    }
    host[hlen] = '\0';

    int ok = host_allowed(host);
    free(host);
    if (!ok) {
        free(t);
        return str_dup("/");
    }

    // Allowed: return the original trimmed URL
    // Caller must free the returned string
    return t;
}

int main(void) {
    const char* tests[5] = {
        "https://example.com/abc",
        "http://sub.example.com:8080/path",
        "https://evil.com",
        "/relative/path",
        "https://notexample.com"
    };
    for (int i = 0; i < 5; ++i) {
        char* result = safe_redirect(tests[i]);
        printf("Input: %s -> Redirect: %s\n", tests[i], result);
        free(result);
    }
    return 0;
}