#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#define MAX_LEN 2048

static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return -1;
}

static char* url_decode(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    size_t oi = 0;
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (c == '%' && i + 2 < len) {
            int h1 = hexVal(s[i + 1]);
            int h2 = hexVal(s[i + 2]);
            if (h1 >= 0 && h2 >= 0) {
                out[oi++] = (char)((h1 << 4) | h2);
                i += 2;
            } else {
                out[oi++] = c;
            }
        } else if (c == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = c;
        }
    }
    out[oi] = '\0';
    return out;
}

static char* get_param(const char* query, const char* key) {
    if (!query || !key) return NULL;
    size_t qlen = strlen(query);
    size_t start = 0;
    while (start <= qlen) {
        size_t end = start;
        while (end < qlen && query[end] != '&') end++;
        size_t pair_len = end - start;
        char* pair = (char*)malloc(pair_len + 1);
        if (!pair) return NULL;
        memcpy(pair, query + start, pair_len);
        pair[pair_len] = '\0';

        char* eq = strchr(pair, '=');
        char* k = NULL;
        char* v = NULL;
        if (eq) {
            *eq = '\0';
            k = url_decode(pair);
            v = url_decode(eq + 1);
        } else {
            k = url_decode(pair);
            v = strdup("");
        }

        int match = (k && strcmp(k, key) == 0);
        free(pair);
        if (match) {
            free(k);
            return v; // caller frees
        }
        free(k);
        free(v);

        if (end == qlen) break;
        start = end + 1;
    }
    return NULL;
}

char* decide_redirect_from_query(const char* query) {
    const char* q = query;
    if (!q) {
        char* root = strdup("/");
        return root;
    }
    if (q[0] == '?') q++;

    char* target = get_param(q, "target");
    if (!target) return strdup("/");

    // Trim whitespace
    char* start = target;
    while (*start && isspace((unsigned char)*start)) start++;
    char* end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    *end = '\0';

    if (*start == '\0' || (size_t)(end - start) > MAX_LEN) {
        free(target);
        return strdup("/");
    }
    if (strchr(start, '\r') || strchr(start, '\n')) {
        free(target);
        return strdup("/");
    }

    // Regex: starts with one or more alphanumerics or hyphens, followed by a dot
    regex_t reg;
    const char* pattern = "^[A-Za-z0-9-]+\\.";
    if (regcomp(&reg, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
        free(target);
        return strdup("/");
    }
    int rx = regexec(&reg, start, 0, NULL, 0);
    regfree(&reg);

    if (rx == 0) {
        // Return a duplicate of the trimmed target
        char* ok = strdup(start);
        free(target);
        return ok;
    } else {
        free(target);
        return strdup("/");
    }
}

int main(void) {
    const char* tests[] = {
        "target=abc.",
        "target=abc.def",
        "foo=1&target=-hyphen.",
        "target=..evil",
        "foo=bar"
    };
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); ++i) {
        char* res = decide_redirect_from_query(tests[i]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("/\n");
        }
    }
    return 0;
}