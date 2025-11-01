#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_URL_LEN 2048
#define RESP_BUF_SZ 8192

static int has_control(const char *s) {
    if (!s) return 1;
    for (const unsigned char *p = (const unsigned char*)s; *p; ++p) {
        if (*p <= 0x1F || *p == 0x7F) return 1;
    }
    return 0;
}

static int url_decode(const char *in, char *out, size_t outsz) {
    if (!in || !out || outsz == 0) return 0;
    size_t oi = 0;
    for (size_t i = 0; in[i] != '\0'; ++i) {
        char c = in[i];
        if (c == '%') {
            if (!in[i+1] || !in[i+2]) return 0;
            char h1 = in[i+1], h2 = in[i+2];
            int hi, lo;
            if (h1 >= '0' && h1 <= '9') hi = h1 - '0';
            else if (h1 >= 'A' && h1 <= 'F') hi = h1 - 'A' + 10;
            else if (h1 >= 'a' && h1 <= 'f') hi = h1 - 'a' + 10;
            else return 0;
            if (h2 >= '0' && h2 <= '9') lo = h2 - '0';
            else if (h2 >= 'A' && h2 <= 'F') lo = h2 - 'A' + 10;
            else if (h2 >= 'a' && h2 <= 'f') lo = h2 - 'a' + 10;
            else return 0;
            if (oi + 1 >= outsz) return 0;
            out[oi++] = (char)((hi << 4) | lo);
            i += 2;
        } else {
            if (oi + 1 >= outsz) return 0;
            out[oi++] = c;
        }
    }
    if (oi >= outsz) return 0;
    out[oi] = '\0';
    return 1;
}

// Returns 1 if valid and fills out_target, else 0 and fills out_msg
int redirect_invalid_url(const char *rawUrl, char *out_msg, size_t out_msg_sz, char *out_target, size_t out_target_sz) {
    if (!rawUrl || !out_msg || !out_target) return 0;
    if (out_msg_sz > 0) out_msg[0] = '\0';
    if (out_target_sz > 0) out_target[0] = '\0';

    size_t len = strlen(rawUrl);
    if (len == 0 || len > MAX_URL_LEN) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    if (has_control(rawUrl)) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    const char *scheme_end = strstr(rawUrl, "://");
    if (!scheme_end) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    size_t scheme_len = (size_t)(scheme_end - rawUrl);
    if (scheme_len == 0 || scheme_len > 10) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    char scheme[12];
    if (scheme_len >= sizeof(scheme)) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    for (size_t i = 0; i < scheme_len; ++i) scheme[i] = (char)tolower((unsigned char)rawUrl[i]);
    scheme[scheme_len] = '\0';
    if (strcmp(scheme, "http") != 0 && strcmp(scheme, "https") != 0) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    const char *host_start = scheme_end + 3;
    if (*host_start == '\0') {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    const char *auth_end = host_start;
    while (*auth_end && *auth_end != '/' && *auth_end != '?' && *auth_end != '#') auth_end++;
    size_t auth_len = (size_t)(auth_end - host_start);
    if (auth_len == 0 || auth_len > 1024) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    // Reject userinfo
    for (size_t i = 0; i < auth_len; ++i) {
        if (host_start[i] == '@') {
            snprintf(out_msg, out_msg_sz, "Invalid URL");
            return 0;
        }
    }
    // Split host:port
    const char *colon = NULL;
    for (size_t i = auth_len; i > 0; --i) {
        if (host_start[i-1] == ':') { colon = host_start + (i-1); break; }
    }
    size_t host_len = colon ? (size_t)(colon - host_start) : auth_len;
    if (host_len == 0 || host_len >= 256) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    char host[256];
    memcpy(host, host_start, host_len);
    host[host_len] = '\0';
    for (size_t i = 0; i < host_len; ++i) host[i] = (char)tolower((unsigned char)host[i]);
    if (strcmp(host, "example.com") != 0) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    // Safe to accept
    if (strlen(rawUrl) + 1 > out_target_sz) {
        snprintf(out_msg, out_msg_sz, "Invalid URL");
        return 0;
    }
    snprintf(out_target, out_target_sz, "%s", rawUrl);
    snprintf(out_msg, out_msg_sz, "OK");
    return 1;
}

static void build_response_for_path(const char *path, char *out_resp, size_t out_resp_sz) {
    const char *prefix = "/redirect/";
    if (strncmp(path, prefix, strlen(prefix)) != 0) {
        snprintf(out_resp, out_resp_sz,
                 "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 10\r\n\r\nNot Found\n");
        return;
    }
    const char *enc = path + strlen(prefix);
    if (*enc == '\0') {
        snprintf(out_resp, out_resp_sz,
                 "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 12\r\n\r\nInvalid URL\n");
        return;
    }
    char decoded[MAX_URL_LEN + 1];
    if (!url_decode(enc, decoded, sizeof(decoded))) {
        snprintf(out_resp, out_resp_sz,
                 "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 12\r\n\r\nInvalid URL\n");
        return;
    }
    char msg[64], target[MAX_URL_LEN + 1];
    if (!redirect_invalid_url(decoded, msg, sizeof msg, target, sizeof target)) {
        snprintf(out_resp, out_resp_sz,
                 "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 12\r\n\r\nInvalid URL\n");
        return;
    }
    if (has_control(target)) {
        snprintf(out_resp, out_resp_sz,
                 "HTTP/1.1 400 Bad Request\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 12\r\n\r\nInvalid URL\n");
        return;
    }
    const char *body = "Redirecting...\n";
    int n = snprintf(out_resp, out_resp_sz,
                     "HTTP/1.1 302 Found\r\nLocation: %s\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: %zu\r\n\r\n%s",
                     target, strlen(body), body);
    if (n < 0 || (size_t)n >= out_resp_sz) {
        // Fallback on overflow
        snprintf(out_resp, out_resp_sz,
                 "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain; charset=utf-8\r\nContent-Length: 12\r\n\r\nServer Error\n");
    }
}

int main(void) {
    // Five test cases for the validator
    const char *tests[] = {
        "https://example.com/path",
        "http://example.com",
        "https://evil.com",
        "https://example.com:8443/",
        "https://user@example.com/"
    };
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        char msg[64], target[MAX_URL_LEN + 1];
        int ok = redirect_invalid_url(tests[i], msg, sizeof msg, target, sizeof target);
        if (ok) {
            printf("[VALID] %s -> %s\n", tests[i], target);
        } else {
            printf("[INVALID] %s -> %s\n", tests[i], msg);
        }
    }

    // Five test cases for the route handler (simulated)
    const char *paths[] = {
        "/redirect/https%3A%2F%2Fexample.com%2Fok",
        "/redirect/http%3A%2F%2Fexample.com",
        "/redirect/https%3A%2F%2Fevil.com",
        "/redirect/https%3A%2F%2Fexample.com%3A443%2Fabc",
        "/notredirect/https%3A%2F%2Fexample.com"
    };
    char resp[RESP_BUF_SZ];
    for (size_t i = 0; i < sizeof(paths)/sizeof(paths[0]); ++i) {
        build_response_for_path(paths[i], resp, sizeof resp);
        // Print status line
        char *eol = strstr(resp, "\r\n");
        if (eol) {
            char status[128];
            size_t slen = (size_t)(eol - resp);
            if (slen >= sizeof status) slen = sizeof status - 1;
            memcpy(status, resp, slen);
            status[slen] = '\0';
            printf("REQ %s => %s\n", paths[i], status);
        } else {
            printf("REQ %s => %s\n", paths[i], resp);
        }
    }

    return 0;
}