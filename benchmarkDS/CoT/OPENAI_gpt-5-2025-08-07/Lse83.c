#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Parse JSON string at *i, decode to newly allocated C string; returns NULL on error.
static char* parse_json_string(const char *s, size_t len, size_t *i) {
    if (*i >= len || s[*i] != '"') return NULL;
    (*i)++;
    // Use a dynamic buffer
    size_t cap = 64, sz = 0;
    char *out = (char*)malloc(cap);
    if (!out) return NULL;
    while (*i < len) {
        char c = s[(*i)++];
        if (c == '"') {
            // null-terminate
            if (sz + 1 > cap) {
                cap *= 2;
                char *tmp = (char*)realloc(out, cap);
                if (!tmp) { free(out); return NULL; }
                out = tmp;
            }
            out[sz] = '\0';
            return out;
        }
        if (c == '\\') {
            if (*i >= len) { free(out); return NULL; }
            char e = s[(*i)++];
            switch (e) {
                case '"': c = '"'; break;
                case '\\': c = '\\'; break;
                case '/': c = '/'; break;
                case 'b': c = '\b'; break;
                case 'f': c = '\f'; break;
                case 'n': c = '\n'; break;
                case 'r': c = '\r'; break;
                case 't': c = '\t'; break;
                case 'u': {
                    if (*i + 4 > len) { free(out); return NULL; }
                    int code = 0;
                    for (int k = 0; k < 4; ++k) {
                        char h = s[(*i)++];
                        code <<= 4;
                        if (h >= '0' && h <= '9') code |= (h - '0');
                        else if (h >= 'a' && h <= 'f') code |= (h - 'a' + 10);
                        else if (h >= 'A' && h <= 'F') code |= (h - 'A' + 10);
                        else { free(out); return NULL; }
                    }
                    c = (char)code;
                    break;
                }
                default: free(out); return NULL;
            }
        }
        if (sz + 1 > cap) {
            cap *= 2;
            char *tmp = (char*)realloc(out, cap);
            if (!tmp) { free(out); return NULL; }
            out = tmp;
        }
        out[sz++] = c;
    }
    free(out);
    return NULL;
}

static int is_ws(char c) {
    return c==' '||c=='\n'||c=='\r'||c=='\t';
}

static void skip_ws_c(const char *s, size_t len, size_t *i) {
    while (*i < len && is_ws(s[*i])) (*i)++;
}

// Parse a non-string literal and return malloc'd substring
static char* parse_literal_c(const char *s, size_t len, size_t *i) {
    size_t start = *i;
    while (*i < len) {
        char c = s[*i];
        if (c == ',' || c == '}' || c == ']' || is_ws(c)) break;
        (*i)++;
    }
    if (start == *i) return NULL;
    size_t l = *i - start;
    char *out = (char*)malloc(l + 1);
    if (!out) return NULL;
    memcpy(out, s + start, l);
    out[l] = '\0';
    return out;
}

// Extract 'name' from JSON payload; returns malloc'd string (caller must free) or NULL if not found/invalid.
char* extract_name_c(const char *payload) {
    if (!payload) return NULL;
    size_t len = strlen(payload);
    size_t i = 0;
    while (i < len) {
        if (payload[i] == '"') {
            char *key = parse_json_string(payload, len, &i);
            if (!key) return NULL;
            skip_ws_c(payload, len, &i);
            if (i < len && payload[i] == ':') {
                i++;
                skip_ws_c(payload, len, &i);
                int is_name = strcmp(key, "name") == 0;
                free(key);
                if (is_name) {
                    if (i < len && payload[i] == '"') {
                        char *val = parse_json_string(payload, len, &i);
                        return val; // may be empty
                    } else {
                        char *lit = parse_literal_c(payload, len, &i);
                        return lit;
                    }
                } else {
                    if (i < len) {
                        if (payload[i] == '"') {
                            char *ignore = parse_json_string(payload, len, &i);
                            if (!ignore) return NULL;
                            free(ignore);
                        } else {
                            char *ignore = parse_literal_c(payload, len, &i);
                            if (ignore) free(ignore);
                        }
                    }
                }
            } else {
                free(key);
            }
        }
        i++;
    }
    return NULL;
}

int main(void) {
    // 5 test cases
    const char *tests[5] = {
        "{\"name\":\"Alice\"}",
        "{\"foo\":\"bar\"}",
        "{\"name\":\"Bob \\\"Builder\\\"\"}",
        "{\"name\":\"\"}",
        "{invalid json"
    };
    for (int t = 0; t < 5; ++t) {
        char *res = extract_name_c(tests[t]);
        if (res) {
            printf("Test %d: %s\n", t + 1, res);
            free(res);
        } else {
            printf("Test %d: null\n", t + 1);
        }
    }
    // Note: Web server not implemented in C here to keep the example concise and secure.
    return 0;
}