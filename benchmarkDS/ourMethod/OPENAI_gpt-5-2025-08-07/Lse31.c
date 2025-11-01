#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#define MAX_URL_LEN 2048

static size_t safe_strnlen_(const char *s, size_t maxlen) {
    size_t n = 0;
    if (!s) return 0;
    while (n < maxlen && s[n] != '\0') n++;
    return n;
}

static void to_lower_inplace(char *s) {
    if (!s) return;
    for (; *s; ++s) {
        *s = (char)tolower((unsigned char)*s);
    }
}

static char* make_result(const char* prefix, const char* content, size_t content_len) {
    size_t prefix_len = safe_strnlen_(prefix, 64);
    if (content_len > MAX_URL_LEN) content_len = MAX_URL_LEN;
    if (prefix_len > SIZE_MAX - content_len - 1) return NULL;
    size_t total = prefix_len + content_len + 1;
    char *out = (char*)malloc(total);
    if (!out) return NULL;
    memcpy(out, prefix, prefix_len);
    if (content && content_len > 0) {
        memcpy(out + prefix_len, content, content_len);
    }
    out[prefix_len + content_len] = '\0';
    return out;
}

char* redirect_url(const char* url) {
    if (url == NULL) {
        return make_result("404 Not Found", NULL, 0);
    }
    size_t raw_len = safe_strnlen_(url, MAX_URL_LEN + 1);
    if (raw_len == 0 || raw_len > MAX_URL_LEN) {
        return make_result("404 Not Found", NULL, 0);
    }

    // Trim leading/trailing whitespace
    const char *begin = url;
    while (*begin && isspace((unsigned char)*begin)) begin++;
    const char *endp = url + raw_len;
    while (endp > begin && isspace((unsigned char)*(endp - 1))) endp--;
    if (endp <= begin) {
        return make_result("404 Not Found", NULL, 0);
    }
    size_t trimmed_len = (size_t)(endp - begin);

    // Work on a temporary buffer (bounded)
    char *u = (char*)malloc(trimmed_len + 1);
    if (!u) return make_result("404 Not Found", NULL, 0);
    memcpy(u, begin, trimmed_len);
    u[trimmed_len] = '\0';

    // Find scheme
    char *scheme_sep = strstr(u, "://");
    if (!scheme_sep) {
        free(u);
        return make_result("404 Not Found", NULL, 0);
    }
    size_t scheme_len = (size_t)(scheme_sep - u);
    if (scheme_len == 0) {
        free(u);
        return make_result("404 Not Found", NULL, 0);
    }

    // Extract scheme in lower-case
    char scheme_buf[8];
    if (scheme_len >= sizeof(scheme_buf)) {
        free(u);
        return make_result("404 Not Found", NULL, 0);
    }
    memcpy(scheme_buf, u, scheme_len);
    scheme_buf[scheme_len] = '\0';
    to_lower_inplace(scheme_buf);
    if (strcmp(scheme_buf, "http") != 0 && strcmp(scheme_buf, "https") != 0) {
        free(u);
        return make_result("404 Not Found", NULL, 0);
    }

    // Authority start after ://
    char *auth_start = scheme_sep + 3;
    if (*auth_start == '\0') {
        free(u);
        return make_result("404 Not Found", NULL, 0);
    }

    // Authority end at first of / ? #
    char *p = auth_start;
    while (*p && *p != '/' && *p != '?' && *p != '#') p++;
    if (p == auth_start) {
        free(u);
        return make_result("404 Not Found", NULL, 0);
    }
    size_t auth_len = (size_t)(p - auth_start);

    // Extract authority
    char *authority = (char*)malloc(auth_len + 1);
    if (!authority) {
        free(u);
        return make_result("404 Not Found", NULL, 0);
    }
    memcpy(authority, auth_start, auth_len);
    authority[auth_len] = '\0';

    // Strip userinfo
    char *last_at = strrchr(authority, '@');
    char *hostport = last_at ? (last_at + 1) : authority;

    // Extract host
    char host[256];
    size_t host_len = 0;
    if (*hostport == '[') {
        char *rb = strchr(hostport, ']');
        if (!rb || rb == hostport + 1) {
            free(authority);
            free(u);
            return make_result("404 Not Found", NULL, 0);
        }
        size_t len = (size_t)(rb - (hostport + 1));
        if (len >= sizeof(host)) len = sizeof(host) - 1;
        memcpy(host, hostport + 1, len);
        host[len] = '\0';
        host_len = len;
    } else {
        // up to colon or end
        char *colon = strchr(hostport, ':');
        size_t len = colon ? (size_t)(colon - hostport) : safe_strnlen_(hostport, sizeof(host));
        if (len >= sizeof(host)) len = sizeof(host) - 1;
        memcpy(host, hostport, len);
        host[len] = '\0';
        host_len = len;
    }

    to_lower_inplace(host);

    char *result = NULL;
    if (host_len == strlen("example.com") && strcmp(host, "example.com") == 0) {
        result = make_result("REDIRECT ", u, safe_strnlen_(u, MAX_URL_LEN));
    } else {
        result = make_result("404 Not Found", NULL, 0);
    }

    free(authority);
    free(u);
    if (!result) {
        // Allocation failure safety
        return make_result("404 Not Found", NULL, 0);
    }
    return result;
}

int main(void) {
    const char* tests[5] = {
        "https://example.com/path",
        "http://example.com:80?a=b",
        "http://sub.example.com/",
        "ftp://example.com/file",
        "https://evil.com/"
    };
    for (int i = 0; i < 5; ++i) {
        char* out = redirect_url(tests[i]);
        if (out) {
            printf("%s\n", out);
            free(out);
        } else {
            printf("404 Not Found\n");
        }
    }
    return 0;
}