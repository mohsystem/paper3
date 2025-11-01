#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <regex.h>

#define MAX_QUERY_LEN 4096
#define MAX_VALUE_LEN 2048

static int is_hex(char c) {
    return isdigit((unsigned char)c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static int hex_val(char c) {
    if (isdigit((unsigned char)c)) return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    return 10 + (c - 'A');
}

// Decode percent-encoding and '+' to space. Writes at most outcap bytes (including null terminator).
// Returns 0 on failure, 1 on success.
static int url_decode_limited(const char *in, size_t inlen, char *out, size_t outcap) {
    if (outcap == 0) return 0;
    size_t oi = 0;
    for (size_t i = 0; i < inlen; i++) {
        if (oi + 1 >= outcap) return 0; // leave space for null
        char c = in[i];
        if (c == '%') {
            if (i + 2 >= inlen || !is_hex(in[i+1]) || !is_hex(in[i+2])) return 0;
            char d = (char)((hex_val(in[i+1]) << 4) | hex_val(in[i+2]));
            out[oi++] = d;
            i += 2;
        } else if (c == '+') {
            out[oi++] = ' ';
        } else {
            out[oi++] = c;
        }
    }
    out[oi] = '\0';
    return 1;
}

// Extract first occurrence of param 'name' from query. Returns malloc'ed decoded value, or NULL.
static char* extract_param_decoded(const char *query, const char *name) {
    size_t qlen = strnlen(query, MAX_QUERY_LEN + 1);
    if (qlen == 0 || qlen > MAX_QUERY_LEN) return NULL;

    size_t name_len = strlen(name);
    // Iterate over pairs split by '&' up to 200 parts
    size_t start = 0;
    int parts = 0;
    while (start <= qlen && parts < 200) {
        size_t end = start;
        while (end < qlen && query[end] != '&') end++;
        size_t pair_len = end - start;
        if (pair_len > 0) {
            const char *pair = query + start;
            size_t offset = 0;
            if (pair[0] == '?') { offset = 1; if (pair_len > 0) pair_len--; }
            // find '='
            size_t i;
            for (i = offset; i < offset + pair_len; i++) {
                if (pair[i] == '=') break;
            }
            if (i < offset + pair_len) {
                size_t key_start = offset;
                size_t key_len = i - offset;
                size_t val_start = i + 1;
                size_t val_len = offset + pair_len - val_start;
                if (key_len == name_len && strncmp(pair + key_start, name, name_len) == 0) {
                    // decode value
                    char *decoded = (char*)calloc(MAX_VALUE_LEN + 1, 1);
                    if (!decoded) return NULL;
                    if (url_decode_limited(pair + val_start, val_len, decoded, MAX_VALUE_LEN + 1)) {
                        return decoded; // caller frees
                    } else {
                        free(decoded);
                        return NULL;
                    }
                }
            }
        }
        if (end == qlen) break;
        start = end + 1;
        parts++;
    }
    return NULL;
}

static int is_safe_ascii(const char *s) {
    for (const unsigned char *p = (const unsigned char*)s; *p; p++) {
        if (*p < 0x20 || *p == 0x7F || *p > 0x7E) return 0;
    }
    return 1;
}

// Returns malloc'ed string with redirect target or "/" on failure. Caller must free.
char* determine_redirect(const char *query_string) {
    if (query_string == NULL) {
        char *root = (char*)malloc(2);
        if (root) { root[0] = '/'; root[1] = '\0'; }
        return root;
    }
    size_t qlen = strnlen(query_string, MAX_QUERY_LEN + 1);
    if (qlen == 0 || qlen > MAX_QUERY_LEN) {
        char *root = (char*)malloc(2);
        if (root) { root[0] = '/'; root[1] = '\0'; }
        return root;
    }

    char *target = extract_param_decoded(query_string, "target");
    if (!target) {
        char *root = (char*)malloc(2);
        if (root) { root[0] = '/'; root[1] = '\0'; }
        return root;
    }

    size_t tlen = strnlen(target, MAX_VALUE_LEN + 1);
    if (tlen == 0 || tlen > MAX_VALUE_LEN || !is_safe_ascii(target)) {
        free(target);
        char *root = (char*)malloc(2);
        if (root) { root[0] = '/'; root[1] = '\0'; }
        return root;
    }

    // Compile regex: ^[A-Za-z0-9-]+\..*
    regex_t re;
    int rc = regcomp(&re, "^[A-Za-z0-9-]+\\..*", REG_EXTENDED | REG_NOSUB);
    if (rc != 0) {
        free(target);
        char *root = (char*)malloc(2);
        if (root) { root[0] = '/'; root[1] = '\0'; }
        return root;
    }
    int match = regexec(&re, target, 0, NULL, 0);
    regfree(&re);

    if (match == 0) {
        // valid - return target
        return target; // caller frees
    } else {
        free(target);
        char *root = (char*)malloc(2);
        if (root) { root[0] = '/'; root[1] = '\0'; }
        return root;
    }
}

int main(void) {
    const char *tests[5] = {
        "target=abc.example.com",           // valid
        "target=abc-",                      // invalid
        "other=1&target=-sub.example.org",  // valid
        "x=1&target=foo.&y=2",              // valid
        "target=%0d%0ahttps://evil.com"     // invalid due to control chars
    };
    for (int i = 0; i < 5; i++) {
        char *res = determine_redirect(tests[i]);
        if (res) {
            printf("Query: %s -> Redirect: %s\n", tests[i], res);
            free(res);
        } else {
            printf("Query: %s -> Redirect: <allocation failure>\n", tests[i]);
        }
    }
    return 0;
}