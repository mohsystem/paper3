#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#if defined(__unix__) || defined(__APPLE__) || defined(__MACH__)
  #include <arpa/inet.h>
  #include <netinet/in.h>
  #define HAVE_INET_PTON 1
#else
  #define HAVE_INET_PTON 0
#endif

static int has_control_or_space(const char* s) {
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (isspace(*p) || *p < 0x20 || *p == 0x7F) return 1;
    }
    return 0;
}

static int is_digits(const char* s) {
    if (!s || !*s) return 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        if (!isdigit(*p)) return 0;
    }
    return 1;
}

static int is_valid_port(const char* s) {
    if (!is_digits(s)) return 0;
    if (s[0] == '0' && s[1] != '\0') return 0;
    char* end = NULL;
    long v = strtol(s, &end, 10);
    if (*end != '\0') return 0;
    return v >= 1 && v <= 65535;
}

static int is_valid_ipv4(const char* host) {
    int len = (int)strlen(host);
    if (len < 7 || len > 15) return 0;
    int dots = 0;
    for (int i = 0; i < len; i++) if (host[i] == '.') dots++;
    if (dots != 3) return 0;

    int parts = 0;
    const char* p = host;
    while (*p) {
        const char* start = p;
        int digits = 0;
        while (*p && *p != '.') {
            if (!isdigit((unsigned char)*p)) return 0;
            digits++;
            if (digits > 3) return 0;
            p++;
        }
        if (digits == 0) return 0;
        if (digits > 1 && start[0] == '0') return 0;
        char buf[4] = {0,0,0,0};
        memcpy(buf, start, (size_t)digits);
        int val = atoi(buf);
        if (val < 0 || val > 255) return 0;
        parts++;
        if (*p == '.') p++;
    }
    return parts == 4;
}

static int is_alnum_dash(char c) {
    return isalnum((unsigned char)c) || c == '-';
}

static int is_valid_domain(const char* host) {
    size_t len = strlen(host);
    if (len == 0 || len > 253) return 0;
    char buf[256 * 4]; // generous buffer
    if (len >= sizeof(buf)) return 0;
    strncpy(buf, host, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    size_t blen = strlen(buf);
    if (blen > 0 && buf[blen - 1] == '.') buf[blen - 1] = '\0';
    if (buf[0] == '\0') return 0;

    const char* p = buf;
    while (*p) {
        const char* start = p;
        size_t labellen = 0;
        while (*p && *p != '.') {
            if (!is_alnum_dash(*p)) return 0;
            p++;
            labellen++;
            if (labellen > 63) return 0;
        }
        if (labellen == 0) return 0;
        if (start[0] == '-' || start[labellen - 1] == '-') return 0;
        if (*p == '.') p++;
    }
    return 1;
}

static int is_valid_ipv6_literal(const char* host) {
#if HAVE_INET_PTON
    unsigned char buf[16];
    return inet_pton(AF_INET6, host, buf) == 1;
#else
    // Minimal sanity check: contains ':' and only hex, ':', or '.' chars
    int has_colon = 0;
    for (const char* p = host; *p; ++p) {
        char c = *p;
        if (c == ':') has_colon = 1;
        if (!(isxdigit((unsigned char)c) || c == ':' || c == '.')) return 0;
    }
    return has_colon;
#endif
}

int is_valid_url(const char* url) {
    if (!url) return 0;
    size_t len = strlen(url);
    if (len == 0 || len > 2048) return 0;
    if (has_control_or_space(url)) return 0;

    const char* scheme_sep = strstr(url, "://");
    if (!scheme_sep || scheme_sep == url) return 0;

    // scheme check
    size_t scheme_len = (size_t)(scheme_sep - url);
    char scheme[16];
    if (scheme_len >= sizeof(scheme)) return 0;
    for (size_t i = 0; i < scheme_len; i++) {
        scheme[i] = (char)tolower((unsigned char)url[i]);
    }
    scheme[scheme_len] = '\0';
    if (strcmp(scheme, "http") != 0 && strcmp(scheme, "https") != 0) return 0;

    const char* auth_start = scheme_sep + 3;
    const char* p = auth_start;
    while (*p && *p != '/' && *p != '?' && *p != '#') p++;
    if (p == auth_start) return 0;
    size_t netloc_len = (size_t)(p - auth_start);
    char netloc[1024];
    if (netloc_len == 0 || netloc_len >= sizeof(netloc)) return 0;
    memcpy(netloc, auth_start, netloc_len);
    netloc[netloc_len] = '\0';

    // disallow credentials
    if (strchr(netloc, '@') != NULL) return 0;

    char host[1024] = {0};
    char port[8] = {0};

    if (netloc[0] == '[') {
        char* rb = strchr(netloc, ']');
        if (!rb) return 0;
        size_t hlen = (size_t)(rb - (netloc + 1));
        if (hlen == 0 || hlen >= sizeof(host)) return 0;
        memcpy(host, netloc + 1, hlen);
        host[hlen] = '\0';
        if (*(rb + 1) != '\0') {
            if (*(rb + 1) != ':') return 0;
            const char* portstr = rb + 2;
            if (strlen(portstr) == 0 || strlen(portstr) >= sizeof(port)) return 0;
            strcpy(port, portstr);
            if (!is_valid_port(port)) return 0;
        }
        if (!is_valid_ipv6_literal(host)) return 0;
    } else {
        // Split host:port (only one colon allowed)
        const char* first_colon = strchr(netloc, ':');
        const char* last_colon = strrchr(netloc, ':');
        if (first_colon && first_colon != last_colon) {
            // multiple colons without brackets -> invalid
            return 0;
        }
        if (first_colon) {
            size_t hlen = (size_t)(first_colon - netloc);
            if (hlen == 0 || hlen >= sizeof(host)) return 0;
            memcpy(host, netloc, hlen);
            host[hlen] = '\0';
            const char* portstr = first_colon + 1;
            if (strlen(portstr) == 0 || strlen(portstr) >= sizeof(port)) return 0;
            strcpy(port, portstr);
            if (!is_valid_port(port)) return 0;
        } else {
            if (strlen(netloc) >= sizeof(host)) return 0;
            strcpy(host, netloc);
        }
        if (host[0] == '\0') return 0;

        int ipv4_candidate = 1;
        for (const char* q = host; *q; ++q) {
            if (!isdigit((unsigned char)*q) && *q != '.') { ipv4_candidate = 0; break; }
        }
        if (ipv4_candidate) {
            if (!is_valid_ipv4(host)) return 0;
        } else {
            if (!is_valid_domain(host)) return 0;
        }
    }
    return 1;
}

int main(void) {
    const char* tests[5] = {
        "https://example.com",
        "http://sub.example.co.uk/path?query=1#frag",
        "https://user:pass@example.com",
        "ftp://example.com",
        "http://256.256.256.256"
    };
    for (int i = 0; i < 5; i++) {
        printf("%s -> %s\n", tests[i], is_valid_url(tests[i]) ? "true" : "false");
    }
    return 0;
}