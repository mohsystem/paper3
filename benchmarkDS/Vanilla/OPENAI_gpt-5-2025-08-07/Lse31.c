#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static char* my_strdup(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, s, len + 1);
    return out;
}

static void parse_host(const char* url, char* host, size_t hostSize) {
    const char* p = strstr(url, "://");
    p = (p ? p + 3 : url);

    // Move past credentials if present
    const char* scan = p;
    while (*scan && *scan != '/' && *scan != '?' && *scan != '#') {
        if (*scan == '@') {
            p = scan + 1;
        }
        scan++;
    }

    const char* end = p;
    while (*end && *end != ':' && *end != '/' && *end != '?' && *end != '#') {
        end++;
    }

    size_t len = (size_t)(end - p);
    if (len >= hostSize) len = hostSize - 1;
    memcpy(host, p, len);
    host[len] = '\0';

    // Lowercase
    for (size_t i = 0; host[i]; ++i) {
        host[i] = (char)tolower((unsigned char)host[i]);
    }
}

char* redirect_url(const char* url) {
    char host[256];
    parse_host(url, host, sizeof(host));
    if (strcmp(host, "example.com") == 0) {
        const char* prefix = "Redirecting to ";
        size_t sz = strlen(prefix) + strlen(url) + 1;
        char* res = (char*)malloc(sz);
        if (!res) return NULL;
        snprintf(res, sz, "%s%s", prefix, url);
        return res;
    } else {
        return my_strdup("404 Not Found");
    }
}

int main(void) {
    const char* tests[5] = {
        "http://example.com",
        "https://example.com/path",
        "http://example.com:8080/abc",
        "https://sub.example.com/home",
        "https://notexample.com"
    };
    for (int i = 0; i < 5; ++i) {
        char* res = redirect_url(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("Memory allocation failure\n");
        }
    }
    return 0;
}