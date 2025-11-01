/*
Chain-of-Through in-code notes:
1) Problem: Provide redirect_invalid_url() and a simulated route.
2) Security: Validate scheme http/https, exact host example.com, reject CRLF, avoid buffer overflow.
3) Secure coding: Careful parsing using indices and bounds.
4) Review: No direct header reflection without validation.
5) Output: Tests included.
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static int contains_crlf(const char* s) {
    for (const char* p = s; *p; ++p) {
        if (*p == '\r' || *p == '\n') return 1;
    }
    return 0;
}

static void tolower_inplace(char* s) {
    for (; *s; ++s) *s = (char)tolower((unsigned char)*s);
}

// Extract scheme and host from URL into provided buffers.
// Returns 1 on success, 0 on failure.
static int extract_scheme_host(const char* url, char* scheme, size_t scheme_sz, char* host, size_t host_sz) {
    const char* p = strstr(url, "://");
    if (!p) return 0;
    size_t s_len = (size_t)(p - url);
    if (s_len == 0 || s_len >= scheme_sz) return 0;
    memcpy(scheme, url, s_len);
    scheme[s_len] = '\0';

    const char* rest = p + 3;
    if (*rest == '\0') return 0;

    // host is up to '/', ':', '?', '#'
    const char* end = rest;
    while (*end && *end != '/' && *end != ':' && *end != '?' && *end != '#') {
        end++;
    }
    size_t h_len = (size_t)(end - rest);
    if (h_len == 0 || h_len >= host_sz) return 0;
    memcpy(host, rest, h_len);
    host[h_len] = '\0';
    return 1;
}

// redirect_invalid_url: returns 1 for redirect allowed, 0 for error.
// If redirect allowed: writes the safe URL into out (copy of input).
// If error: writes error message into out.
int redirect_invalid_url(const char* inputUrl, char* out, size_t out_sz) {
    if (!inputUrl || !*inputUrl) {
        snprintf(out, out_sz, "Missing URL");
        return 0;
    }
    if (contains_crlf(inputUrl)) {
        snprintf(out, out_sz, "Invalid characters in URL");
        return 0;
    }
    char scheme[16], host[256];
    if (!extract_scheme_host(inputUrl, scheme, sizeof(scheme), host, sizeof(host))) {
        snprintf(out, out_sz, "URL must include scheme and host");
        return 0;
    }
    // validate scheme
    char lscheme[16];
    strncpy(lscheme, scheme, sizeof(lscheme) - 1);
    lscheme[sizeof(lscheme) - 1] = '\0';
    tolower_inplace(lscheme);
    if (strcmp(lscheme, "http") != 0 && strcmp(lscheme, "https") != 0) {
        snprintf(out, out_sz, "Only http and https schemes are allowed");
        return 0;
    }
    char lhost[256];
    strncpy(lhost, host, sizeof(lhost) - 1);
    lhost[sizeof(lhost) - 1] = '\0';
    tolower_inplace(lhost);
    if (strcmp(lhost, "example.com") != 0) {
        snprintf(out, out_sz, "URL must have domain example.com");
        return 0;
    }
    if (contains_crlf(inputUrl)) {
        snprintf(out, out_sz, "Invalid URL");
        return 0;
    }
    // Safe: echo original URL as redirection target
    snprintf(out, out_sz, "%s", inputUrl);
    return 1;
}

// Simulated route handler
void handle_request(const char* urlParam) {
    char buf[512];
    int ok = redirect_invalid_url(urlParam, buf, sizeof(buf));
    if (ok) {
        printf("HTTP/1.1 302 Found\r\nLocation: %s\r\n\r\n", buf);
    } else {
        printf("HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain\r\n\r\n%s\n", buf);
    }
}

int main(void) {
    const char* tests[5] = {
        "https://example.com/path",
        "http://example.com",
        "https://sub.example.com/path",  // invalid: subdomain
        "ftp://example.com/resource",    // invalid: scheme
        "https://evil.com/path"          // invalid: host
    };
    printf("C tests for redirect_invalid_url:\n");
    for (int i = 0; i < 5; ++i) {
        char out[512];
        int ok = redirect_invalid_url(tests[i], out, sizeof(out));
        if (ok) {
            printf("OK redirect -> %s\n", out);
        } else {
            printf("ERROR -> %s\n", out);
        }
    }
    printf("\nSimulated route responses:\n");
    handle_request("https://example.com/ok");
    handle_request("https://evil.com/bad");
    return 0;
}