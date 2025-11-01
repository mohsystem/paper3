#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void str_tolower(char *s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

static char hex_to_char(char h, char l) {
    int hi = isdigit((unsigned char)h) ? h - '0' : (tolower((unsigned char)h) - 'a' + 10);
    int lo = isdigit((unsigned char)l) ? l - '0' : (tolower((unsigned char)l) - 'a' + 10);
    return (char)((hi << 4) | lo);
}

static char* url_decode(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '%' && i + 2 < len && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])) {
            out[oi++] = hex_to_char(s[i+1], s[i+2]);
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

static char* get_query_param(const char* query, const char* key) {
    if (!query) return NULL;
    size_t keylen = strlen(key);
    const char* p = query;
    while (*p) {
        const char* amp = strchr(p, '&');
        size_t seglen = amp ? (size_t)(amp - p) : strlen(p);
        const char* eq = memchr(p, '=', seglen);
        if (eq) {
            size_t klen = (size_t)(eq - p);
            char* k = (char*)malloc(klen + 1);
            memcpy(k, p, klen); k[klen] = '\0';
            char* kdec = url_decode(k);
            free(k);
            int match = (kdec && strcmp(kdec, key) == 0);
            free(kdec);
            if (match) {
                size_t vlen = seglen - (klen + 1);
                char* v = (char*)malloc(vlen + 1);
                memcpy(v, eq + 1, vlen); v[vlen] = '\0';
                char* vdec = url_decode(v);
                free(v);
                return vdec; // caller frees
            }
        } else {
            char* k = (char*)malloc(seglen + 1);
            memcpy(k, p, seglen); k[seglen] = '\0';
            char* kdec = url_decode(k);
            free(k);
            if (kdec && strcmp(kdec, key) == 0) {
                free(kdec);
                char* empty = (char*)malloc(1);
                empty[0] = '\0';
                return empty; // empty value
            }
            free(kdec);
        }
        if (!amp) break;
        p = amp + 1;
    }
    return NULL;
}

static int extract_host(const char* url, char* host_out, size_t host_out_size) {
    const char* p = strstr(url, "://");
    if (!p) return 0;
    size_t scheme_len = (size_t)(p - url);
    if (scheme_len == 0) return 0;
    char scheme[16];
    if (scheme_len >= sizeof(scheme)) return 0;
    memcpy(scheme, url, scheme_len);
    scheme[scheme_len] = '\0';
    for (size_t i = 0; i < scheme_len; ++i) scheme[i] = (char)tolower((unsigned char)scheme[i]);
    if (strcmp(scheme, "http") != 0 && strcmp(scheme, "https") != 0) return 0;
    const char* host_start = p + 3;
    if (*host_start == '\0') return 0;
    const char* host_end = host_start;
    while (*host_end && *host_end != ':' && *host_end != '/' && *host_end != '?' && *host_end != '#') {
        host_end++;
    }
    size_t host_len = (size_t)(host_end - host_start);
    if (host_len == 0 || host_len >= host_out_size) return 0;
    memcpy(host_out, host_start, host_len);
    host_out[host_len] = '\0';
    return 1;
}

static int redirect_invalid_url(const char* url, char* out_msg, size_t out_msg_size, int* status_out) {
    if (!url || !*url) {
        snprintf(out_msg, out_msg_size, "%s", "Missing url parameter");
        *status_out = 400;
        return 0;
    }
    char host[256];
    if (!extract_host(url, host, sizeof(host))) {
        snprintf(out_msg, out_msg_size, "%s", "Invalid URL");
        *status_out = 400;
        return 0;
    }
    for (char* p = host; *p; ++p) *p = (char)tolower((unsigned char)*p);
    if (strcmp(host, "example.com") == 0) {
        snprintf(out_msg, out_msg_size, "%s", url);
        *status_out = 302;
        return 1;
    } else {
        snprintf(out_msg, out_msg_size, "%s", "Invalid domain");
        *status_out = 400;
        return 0;
    }
}

int main(void) {
    const char* req_method = getenv("REQUEST_METHOD");
    if (req_method) {
        const char* qs = getenv("QUERY_STRING");
        char* url = get_query_param(qs ? qs : "", "url");
        char msg[1024];
        int status = 200;
        int ok = redirect_invalid_url(url ? url : "", msg, sizeof(msg), &status);
        if (ok) {
            printf("Status: 302 Found\r\n");
            printf("Location: %s\r\n", msg);
            printf("Content-Type: text/plain\r\n\r\n");
            printf("Redirecting to %s\n", msg);
        } else {
            printf("Status: 400 Bad Request\r\n");
            printf("Content-Type: text/plain\r\n\r\n");
            printf("%s\n", msg);
        }
        if (url) free(url);
        return 0;
    }

    // 5 test cases
    const char* tests[] = {
        "https://example.com/path",
        "http://example.com",
        "https://sub.example.com",
        "https://example.org",
        "not a url"
    };
    for (int i = 0; i < 5; ++i) {
        char msg[1024];
        int status = 0;
        int ok = redirect_invalid_url(tests[i], msg, sizeof(msg), &status);
        printf("Test: %s -> ok=%s status=%d result=%s\n", tests[i], ok ? "true" : "false", status, msg);
    }
    return 0;
}