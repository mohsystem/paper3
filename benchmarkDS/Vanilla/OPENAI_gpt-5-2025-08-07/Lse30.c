#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* my_strdup(const char* s) {
    size_t len = strlen(s);
    char* d = (char*)malloc(len + 1);
    if (!d) return NULL;
    memcpy(d, s, len + 1);
    return d;
}

static char* url_decode(const char* s) {
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        if (s[i] == '%' && i + 2 < len && isxdigit((unsigned char)s[i+1]) && isxdigit((unsigned char)s[i+2])) {
            char hex[3] = { s[i+1], s[i+2], '\0' };
            out[oi++] = (char)strtol(hex, NULL, 16);
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

static int matches_prefix_rule(const char* s) {
    // must start with one or more [A-Za-z0-9-] followed by '.'
    if (!s || !*s) return 0;
    int count = 0;
    const unsigned char* p = (const unsigned char*)s;
    while (*p && (isalnum(*p) || *p == '-')) {
        ++count;
        ++p;
    }
    if (count >= 1 && *p == '.') {
        return 1; // no need to require chars after '.'
    }
    return 0;
}

static char* extract_target(const char* query) {
    if (!query) return NULL;
    const char* p = query;
    while (*p) {
        if ((p == query || *(p - 1) == '&') && strncmp(p, "target=", 7) == 0) {
            p += 7;
            const char* end = strchr(p, '&');
            size_t len = end ? (size_t)(end - p) : strlen(p);
            char* val = (char*)malloc(len + 1);
            if (!val) return NULL;
            memcpy(val, p, len);
            val[len] = '\0';
            return val;
        }
        ++p;
    }
    return NULL;
}

char* get_redirect(const char* query) {
    char* raw = extract_target(query);
    if (!raw) {
        return my_strdup("/");
    }
    char* decoded = url_decode(raw);
    free(raw);
    if (!decoded) return my_strdup("/");
    int ok = matches_prefix_rule(decoded);
    if (ok) {
        // return decoded as is
        return decoded;
    } else {
        free(decoded);
        return my_strdup("/");
    }
}

int main(void) {
    const char* tests[] = {
        "target=sub.example.com",
        "target=http%3A%2F%2Fevil.com",
        "target=foo.bar/baz",
        "page=home",
        "target=foo."
    };
    for (int i = 0; i < 5; ++i) {
        char* r = get_redirect(tests[i]);
        if (r) {
            printf("%s\n", r);
            free(r);
        } else {
            printf("\n");
        }
    }
    return 0;
}