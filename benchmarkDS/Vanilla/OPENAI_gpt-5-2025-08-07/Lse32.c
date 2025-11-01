#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Core logic function: accepts a URL string and returns redirect decision and message.
// redirect == 1 => message is the redirect target
// redirect == 0 => message is an error message
typedef struct {
    int redirect;
    char message[512];
} RedirectResult;

RedirectResult redirect_invalid_url(const char* url) {
    RedirectResult res;
    res.redirect = 0;
    res.message[0] = '\0';

    if (url == NULL || url[0] == '\0') {
        snprintf(res.message, sizeof(res.message), "Missing URL");
        return res;
    }

    const char* scheme_sep = strstr(url, "://");
    if (!scheme_sep || scheme_sep == url) {
        snprintf(res.message, sizeof(res.message), "Invalid URL");
        return res;
    }

    const char* host_start = scheme_sep + 3;
    if (*host_start == '\0') {
        snprintf(res.message, sizeof(res.message), "Invalid URL");
        return res;
    }

    // host ends at first of / : ? #
    const char* p = host_start;
    while (*p && *p != '/' && *p != ':' && *p != '?' && *p != '#') {
        p++;
    }
    if (p == host_start) {
        snprintf(res.message, sizeof(res.message), "Invalid URL");
        return res;
    }

    char host[256];
    size_t len = (size_t)(p - host_start);
    if (len >= sizeof(host)) len = sizeof(host) - 1;
    memcpy(host, host_start, len);
    host[len] = '\0';

    // lowercase host
    for (size_t i = 0; i < len; ++i) host[i] = (char)tolower((unsigned char)host[i]);

    if (strcmp(host, "example.com") == 0) {
        res.redirect = 1;
        snprintf(res.message, sizeof(res.message), "%s", url);
    } else {
        snprintf(res.message, sizeof(res.message), "URL must be from example.com");
    }

    return res;
}

int main(void) {
    // This C program demonstrates the core logic with 5 test cases.
    // It is not a full web server; integrate redirect_invalid_url into your server/router as needed.
    const char* tests[5] = {
        "https://example.com/",
        "http://example.com/blog",
        "https://evil.com/hack",
        "notaurl",
        "https://sub.example.com/"
    };
    for (int i = 0; i < 5; ++i) {
        RedirectResult r = redirect_invalid_url(tests[i]);
        if (r.redirect) {
            printf("Test %d: 302 redirect to %s\n", i+1, r.message);
        } else {
            printf("Test %d: 400 %s\n", i+1, r.message);
        }
    }
    return 0;
}