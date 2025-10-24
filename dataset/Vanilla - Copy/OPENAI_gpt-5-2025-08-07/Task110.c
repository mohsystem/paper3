#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static int is_alnum_c(char c) {
    return isdigit((unsigned char)c) || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static int is_hex_c(char c) {
    return isdigit((unsigned char)c) ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static int is_valid_port(const char* s) {
    if (!s || !*s) return 0;
    int len = 0;
    int val = 0;
    while (*s) {
        if (!isdigit((unsigned char)*s)) return 0;
        val = val * 10 + (*s - '0');
        len++;
        if (len > 5) return 0;
        s++;
    }
    return val >= 1 && val <= 65535;
}

static int is_valid_ipv4(const char* s) {
    int dots = 0;
    for (const char* p = s; *p; ++p) if (*p == '.') dots++;
    if (dots != 3) return 0;

    int parts = 0;
    const char* p = s;
    while (*p) {
        if (parts > 3) return 0;
        int len = 0, val = 0;
        if (!isdigit((unsigned char)*p)) return 0;
        const char* start = p;
        while (*p && isdigit((unsigned char)*p)) {
            val = val * 10 + (*p - '0');
            len++;
            if (len > 3) return 0;
            p++;
        }
        if (len > 1 && *start == '0') return 0;
        if (val < 0 || val > 255) return 0;
        parts++;
        if (*p == '.') p++;
        else if (*p == '\0') break;
        else return 0;
    }
    return parts == 4;
}

static int is_valid_domain(const char* host) {
    int len = (int)strlen(host);
    if (len == 0 || len > 253) return 0;
    if (host[0] == '.' || host[len - 1] == '.') return 0;

    int i = 0;
    while (i < len) {
        int start = i;
        int label_len = 0;
        while (i < len && host[i] != '.') {
            char c = host[i];
            if (!(is_alnum_c(c) || c == '-')) return 0;
            label_len++;
            if (label_len > 63) return 0;
            i++;
        }
        if (label_len < 1) return 0;
        if (!is_alnum_c(host[start]) || !is_alnum_c(host[i - 1])) return 0;
        if (i < len && host[i] == '.') i++;
    }
    return 1;
}

static int is_valid_ipv6(const char* s) {
    if (!s || !*s) return 0;

    char buf[512];
    if (strlen(s) >= sizeof(buf)) return 0;
    strcpy(buf, s);

    int max_hextets = 8;

    // Handle embedded IPv4
    if (strchr(buf, '.') != NULL) {
        char* last_colon = strrchr(buf, ':');
        if (!last_colon) return 0;
        char ipv4part[64];
        strcpy(ipv4part, last_colon + 1);
        if (!is_valid_ipv4(ipv4part)) return 0;
        *last_colon = '\0';
        max_hextets = 6;
    }

    // Count "::"
    char* first_dc = strstr(buf, "::");
    if (first_dc) {
        // Ensure only one occurrence
        char* second_dc = strstr(first_dc + 2, "::");
        if (second_dc) return 0;
    }

    int hextet_count = 0;
    if (first_dc) {
        // Left side
        if (first_dc != buf) {
            char left[512];
            int l = (int)(first_dc - buf);
            strncpy(left, buf, l);
            left[l] = '\0';
            char* token = strtok(left, ":");
            while (token) {
                int k = (int)strlen(token);
                if (k < 1 || k > 4) return 0;
                for (int i = 0; i < k; i++) if (!is_hex_c(token[i])) return 0;
                hextet_count++;
                token = strtok(NULL, ":");
            }
        }
        // Right side
        char right[512];
        strcpy(right, first_dc + 2);
        if (right[0] != '\0') {
            char* token = strtok(right, ":");
            while (token) {
                int k = (int)strlen(token);
                if (k < 1 || k > 4) return 0;
                for (int i = 0; i < k; i++) if (!is_hex_c(token[i])) return 0;
                hextet_count++;
                token = strtok(NULL, ":");
            }
        }
        if (hextet_count >= max_hextets) return 0;
        return 1;
    } else {
        // No compression
        if (buf[0] == '\0') return 0;
        char tmp[512];
        strcpy(tmp, buf);
        char* token = strtok(tmp, ":");
        while (token) {
            int k = (int)strlen(token);
            if (k < 1 || k > 4) return 0;
            for (int i = 0; i < k; i++) if (!is_hex_c(token[i])) return 0;
            hextet_count++;
            token = strtok(NULL, ":");
        }
        return hextet_count == max_hextets;
    }
}

int validateURL(const char* url) {
    if (!url || !*url) return 0;
    for (const char* p = url; *p; ++p) {
        if (isspace((unsigned char)*p)) return 0;
    }

    // Find scheme
    const char* scheme_sep = strstr(url, "://");
    if (!scheme_sep || scheme_sep == url) return 0;
    int scheme_len = (int)(scheme_sep - url);

    // Lowercase compare
    char scheme[16];
    if (scheme_len >= (int)sizeof(scheme)) return 0;
    for (int i = 0; i < scheme_len; i++) {
        char c = url[i];
        if (!isalpha((unsigned char)c)) return 0;
        scheme[i] = (char)tolower((unsigned char)c);
    }
    scheme[scheme_len] = '\0';
    if (strcmp(scheme, "http") != 0 && strcmp(scheme, "https") != 0 && strcmp(scheme, "ftp") != 0) return 0;

    const char* p = scheme_sep + 3;
    if (*p == '\0') return 0;

    // Host and optional port
    char host[512] = {0};
    char port_str[16] = {0};

    if (*p == '[') {
        const char* close = strchr(p + 1, ']');
        if (!close) return 0;
        int hlen = (int)(close - (p + 1));
        if (hlen <= 0 || hlen >= (int)sizeof(host)) return 0;
        strncpy(host, p + 1, hlen);
        host[hlen] = '\0';
        if (!is_valid_ipv6(host)) return 0;
        p = close + 1;
        if (*p == ':') {
            p++;
            const char* start = p;
            int len = 0;
            while (*p && isdigit((unsigned char)*p)) { p++; len++; }
            if (len == 0 || len >= (int)sizeof(port_str)) return 0;
            strncpy(port_str, start, len);
            port_str[len] = '\0';
            if (!is_valid_port(port_str)) return 0;
        }
    } else {
        const char* start = p;
        while (*p && *p != ':' && *p != '/' && *p != '?' && *p != '#') p++;
        int hlen = (int)(p - start);
        if (hlen <= 0 || hlen >= (int)sizeof(host)) return 0;
        strncpy(host, start, hlen);
        host[hlen] = '\0';
        if (strchr(host, ':') != NULL) return 0; // IPv6 must be bracketed
        if (!(is_valid_ipv4(host) || is_valid_domain(host) || strcasecmp(host, "localhost") == 0)) return 0;

        if (*p == ':') {
            p++;
            const char* ps = p;
            int len = 0;
            while (*p && isdigit((unsigned char)*p)) { p++; len++; }
            if (len == 0 || len >= (int)sizeof(port_str)) return 0;
            strncpy(port_str, ps, len);
            port_str[len] = '\0';
            if (!is_valid_port(port_str)) return 0;
        }
    }

    // The rest can be anything non-whitespace (already checked)
    return 1;
}

int main(void) {
    const char* tests[5] = {
        "https://example.com",
        "http://sub.domain.co.uk:8080/path?query=1#frag",
        "ftp://192.168.1.1/resource",
        "https://[2001:db8::1]/index.html",
        "http://-invalid-.com"
    };
    for (int i = 0; i < 5; i++) {
        printf("%s -> %s\n", tests[i], validateURL(tests[i]) ? "true" : "false");
    }
    return 0;
}