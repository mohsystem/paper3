/* Steps 1-5: Problem understanding, security requirements, secure generation, review, secure output (summarized) */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_URL_LEN 2048

static int has_no_whitespace_or_ctl(const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (isspace(*p) || *p < 32) return 0;
    }
    return 1;
}

static int is_digits(const char* s) {
    if (!s || *s == '\0') return 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (!isdigit(*p)) return 0;
    }
    return 1;
}

static int is_valid_ipv4(const char* h) {
    int dots = 0;
    int num = 0;
    int len = 0;
    int segs = 0;
    const char* p = h;
    const char* seg_start = h;

    while (1) {
        if (*p == '.' || *p == '\0') {
            len = (int)(p - seg_start);
            if (len <= 0 || len > 3) return 0;
            for (const char* q = seg_start; q < p; ++q) {
                if (!isdigit((unsigned char)*q)) return 0;
            }
            if (len > 1 && *seg_start == '0') return 0;
            num = atoi(seg_start);
            if (num < 0 || num > 255) return 0;
            segs++;
            if (*p == '\0') break;
            dots++;
            if (dots > 3) return 0;
            seg_start = p + 1;
            if (*seg_start == '\0') return 0;
        }
        p++;
    }
    return segs == 4 && dots == 3;
}

static int is_valid_ipv6(const char* h) {
    int has_colon = 0;
    int dbl = 0;
    for (size_t i = 0; h[i]; ++i) {
        unsigned char c = (unsigned char)h[i];
        int ok = isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') || c == ':' || c == '.';
        if (!ok) return 0;
        if (c == ':') {
            has_colon = 1;
            if (h[i+1] == ':') dbl++;
        }
    }
    if (!has_colon) return 0;
    if (dbl > 1) return 0;
    return 1;
}

static int is_alnum_dash(char c) {
    return isalnum((unsigned char)c) || c == '-';
}

static int is_valid_domain(const char* h) {
    size_t len = strlen(h);
    if (len == 0 || len > 253) return 0;
    size_t i = 0;
    while (i < len) {
        size_t start = i;
        size_t label_len = 0;
        while (i < len && h[i] != '.') {
            if (!is_alnum_dash(h[i])) return 0;
            i++;
            label_len++;
            if (label_len > 63) return 0;
        }
        if (label_len == 0) return 0;
        if (h[start] == '-' || h[start + label_len - 1] == '-') return 0;
        if (i < len && h[i] == '.') {
            i++; // skip dot
            if (i == len) return 0; // no empty last label
        }
    }
    return 1;
}

int is_valid_url(const char* url) {
    if (url == NULL) return 0;
    size_t n = strlen(url);
    if (n == 0 || n > MAX_URL_LEN) return 0;
    if (!has_no_whitespace_or_ctl(url)) return 0;

    const char* p = strstr(url, "://");
    if (!p || p == url) return 0;

    // scheme
    size_t scheme_len = (size_t)(p - url);
    char scheme[16];
    if (scheme_len >= sizeof(scheme)) return 0;
    for (size_t i = 0; i < scheme_len; ++i) {
        char c = url[i];
        if (!isalpha((unsigned char)c)) return 0;
        scheme[i] = (char)tolower((unsigned char)c);
    }
    scheme[scheme_len] = '\0';
    if (strcmp(scheme, "http") != 0 && strcmp(scheme, "https") != 0) return 0;

    const char* auth_start = p + 3;
    const char* path_start = strpbrk(auth_start, "/?#");
    const char* auth_end = path_start ? path_start : url + n;
    if (auth_end <= auth_start) return 0;

    // userinfo disallowed
    for (const char* q = auth_start; q < auth_end; ++q) {
        if (*q == '@') return 0;
    }

    char host[256] = {0};
    int port = -1;

    if (*auth_start == '[') {
        const char* close = memchr(auth_start, ']', (size_t)(auth_end - auth_start));
        if (!close) return 0;
        size_t hlen = (size_t)(close - (auth_start + 1));
        if (hlen == 0 || hlen >= sizeof(host)) return 0;
        memcpy(host, auth_start + 1, hlen);
        host[hlen] = '\0';
        const char* after = close + 1;
        if (after < auth_end) {
            if (*after != ':') return 0;
            const char* port_start = after + 1;
            if (port_start >= auth_end) return 0;
            char portbuf[8] = {0};
            size_t plen = (size_t)(auth_end - port_start);
            if (plen == 0 || plen >= sizeof(portbuf)) return 0;
            for (size_t i = 0; i < plen; ++i) {
                if (!isdigit((unsigned char)port_start[i])) return 0;
                portbuf[i] = port_start[i];
            }
            portbuf[plen] = '\0';
            long pnum = strtol(portbuf, NULL, 10);
            if (pnum < 1 || pnum > 65535) return 0;
            port = (int)pnum;
        }
        if (!is_valid_ipv6(host)) return 0;
    } else {
        const char* colon = NULL;
        for (const char* q = auth_start; q < auth_end; ++q) {
            if (*q == ':') colon = q;
        }
        if (colon && strchr(auth_start, ':') == colon) {
            size_t hlen = (size_t)(colon - auth_start);
            if (hlen == 0 || hlen >= sizeof(host)) return 0;
            memcpy(host, auth_start, hlen);
            host[hlen] = '\0';
            const char* port_start = colon + 1;
            if (port_start >= auth_end) return 0;
            char portbuf[8] = {0};
            size_t plen = (size_t)(auth_end - port_start);
            if (plen == 0 || plen >= sizeof(portbuf)) return 0;
            for (size_t i = 0; i < plen; ++i) {
                if (!isdigit((unsigned char)port_start[i])) return 0;
                portbuf[i] = port_start[i];
            }
            portbuf[plen] = '\0';
            long pnum = strtol(portbuf, NULL, 10);
            if (pnum < 1 || pnum > 65535) return 0;
            port = (int)pnum;
        } else {
            size_t hlen = (size_t)(auth_end - auth_start);
            if (hlen == 0 || hlen >= sizeof(host)) return 0;
            memcpy(host, auth_start, hlen);
            host[hlen] = '\0';
        }

        if (strcasecmp(host, "localhost") == 0) {
            // ok
        } else if (is_valid_ipv4(host)) {
            // ok
        } else if (strchr(host, ':') && is_valid_ipv6(host)) { // unlikely without brackets; still check
            // ok
        } else if (!is_valid_domain(host)) {
            return 0;
        }
    }

    (void)port; // port validated if present
    return 1;
}

int main(void) {
    const char* tests[5] = {
        "https://example.com",
        "http://sub.example.co.uk:8080/path?query=1#frag",
        "ftp://example.com",
        "https://user:pass@example.com",
        "http://-bad.com"
    };
    for (int i = 0; i < 5; ++i) {
        printf("%s -> %s\n", tests[i], is_valid_url(tests[i]) ? "true" : "false");
    }
    return 0;
}