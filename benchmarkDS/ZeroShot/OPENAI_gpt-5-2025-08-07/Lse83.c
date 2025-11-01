#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BODY_SIZE (10 * 1024)

static const char* skip_ws(const char* p) {
    while (*p == ' ' || *p == '\n' || *p == '\r' || *p == '\t') p++;
    return p;
}

static int hexval(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static void append_utf8(unsigned int code, char** buf, size_t* len, size_t* cap) {
    char tmp[4];
    size_t n = 0;
    if (code <= 0x7F) { tmp[0] = (char)code; n = 1; }
    else if (code <= 0x7FF) {
        tmp[0] = (char)(0xC0 | ((code >> 6) & 0x1F));
        tmp[1] = (char)(0x80 | (code & 0x3F));
        n = 2;
    } else {
        tmp[0] = (char)(0xE0 | ((code >> 12) & 0x0F));
        tmp[1] = (char)(0x80 | ((code >> 6) & 0x3F));
        tmp[2] = (char)(0x80 | (code & 0x3F));
        n = 3;
    }
    if (*len + n + 1 > *cap) {
        *cap = (*cap + n + 16) * 2;
        *buf = (char*)realloc(*buf, *cap);
    }
    for (size_t i = 0; i < n; ++i) (*buf)[(*len)++] = tmp[i];
    (*buf)[*len] = '\0';
}

static char* parse_json_string(const char* s, const char** next) {
    if (*s != '"') return NULL;
    s++;
    size_t cap = 64, len = 0;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    out[0] = '\0';
    while (*s) {
        char c = *s;
        if (c == '"') {
            s++;
            *next = s;
            return out;
        } else if (c == '\\') {
            s++;
            if (!*s) break;
            char esc = *s++;
            switch (esc) {
                case '"': case '\\': case '/': {
                    if (len + 2 > cap) { cap *= 2; out = (char*)realloc(out, cap); }
                    out[len++] = esc; out[len] = '\0';
                } break;
                case 'b': { if (len + 2 > cap) { cap *= 2; out = (char*)realloc(out, cap); } out[len++] = '\b'; out[len]='\0'; } break;
                case 'f': { if (len + 2 > cap) { cap *= 2; out = (char*)realloc(out, cap); } out[len++] = '\f'; out[len]='\0'; } break;
                case 'n': { if (len + 2 > cap) { cap *= 2; out = (char*)realloc(out, cap); } out[len++] = '\n'; out[len]='\0'; } break;
                case 'r': { if (len + 2 > cap) { cap *= 2; out = (char*)realloc(out, cap); } out[len++] = '\r'; out[len]='\0'; } break;
                case 't': { if (len + 2 > cap) { cap *= 2; out = (char*)realloc(out, cap); } out[len++] = '\t'; out[len]='\0'; } break;
                case 'u': {
                    if (!isxdigit((unsigned char)s[0]) || !isxdigit((unsigned char)s[1]) ||
                        !isxdigit((unsigned char)s[2]) || !isxdigit((unsigned char)s[3])) {
                        free(out); return NULL;
                    }
                    int a = hexval(s[0]), b = hexval(s[1]), c1 = hexval(s[2]), d = hexval(s[3]);
                    if (a < 0 || b < 0 || c1 < 0 || d < 0) { free(out); return NULL; }
                    unsigned int code = (unsigned int)((a<<12) | (b<<8) | (c1<<4) | d);
                    append_utf8(code, &out, &len, &cap);
                    s += 4;
                } break;
                default:
                    free(out); return NULL;
            }
        } else {
            if (len + 2 > cap) { cap *= 2; out = (char*)realloc(out, cap); }
            out[len++] = c; out[len] = '\0';
            s++;
        }
    }
    free(out);
    return NULL;
}

static const char* skip_json_value(const char* s) {
    s = skip_ws(s);
    if (*s == '"') {
        const char* nx = NULL;
        char* tmp = parse_json_string(s, &nx);
        if (!tmp) return NULL;
        free(tmp);
        return nx;
    } else if (*s == '{') {
        int depth = 1; s++;
        while (*s && depth > 0) {
            if (*s == '"') {
                const char* nx = NULL;
                char* tmp = parse_json_string(s, &nx);
                if (!tmp) return NULL;
                free(tmp);
                s = nx;
            } else if (*s == '{') { depth++; s++; }
            else if (*s == '}') { depth--; s++; }
            else s++;
        }
        return depth == 0 ? s : NULL;
    } else if (*s == '[') {
        int depth = 1; s++;
        while (*s && depth > 0) {
            if (*s == '"') {
                const char* nx = NULL;
                char* tmp = parse_json_string(s, &nx);
                if (!tmp) return NULL;
                free(tmp);
                s = nx;
            } else if (*s == '[') { depth++; s++; }
            else if (*s == ']') { depth--; s++; }
            else s++;
        }
        return depth == 0 ? s : NULL;
    } else {
        const char* p = s;
        while (*p) {
            char ch = *p;
            if (isalnum((unsigned char)ch) || ch == '+' || ch == '-' || ch == '.' || ch == 'e' || ch == 'E') p++;
            else break;
        }
        if (p == s) return NULL;
        return p;
    }
}

char* handle_request(const char* content_type, const unsigned char* body, size_t body_len) {
    if (!content_type || !body) return NULL;
    // lowercase check for application/json prefix
    size_t i;
    const char* prefix = "application/json";
    for (i = 0; prefix[i]; ++i) {
        char c1 = (char)tolower((unsigned char)content_type[i]);
        if (prefix[i] == '\0') break;
        if (c1 == '\0') return NULL;
        if (c1 != prefix[i]) return NULL;
    }
    if (body_len == 0 || body_len > MAX_BODY_SIZE) return NULL;

    // Ensure null-terminated copy
    char* json = (char*)malloc(body_len + 1);
    if (!json) return NULL;
    memcpy(json, body, body_len);
    json[body_len] = '\0';

    const char* p = json;
    while (*p) {
        p = skip_ws(p);
        if (*p == '"') {
            const char* after_key = NULL;
            char* key = parse_json_string(p, &after_key);
            if (!key) { free(json); return NULL; }
            const char* q = skip_ws(after_key);
            if (*q == ':') {
                q++;
                q = skip_ws(q);
                if (strcmp(key, "name") == 0) {
                    if (*q == '"') {
                        const char* after_val = NULL;
                        char* val = parse_json_string(q, &after_val);
                        free(key);
                        free(json);
                        return val; // caller must free
                    } else {
                        // not a string; ignore
                    }
                }
                const char* skipped = skip_json_value(q);
                if (!skipped) { free(key); free(json); return NULL; }
                p = skipped;
            } else {
                p = q;
            }
            free(key);
        } else {
            p++;
        }
    }
    free(json);
    return NULL;
}

int main(void) {
    const char* ct = "application/json";
    const char* tests[] = {
        "{\"name\":\"Alice\"}",
        "{\"id\":123,\"name\":\"Bob \\\\ \\\"The Builder\\\"\",\"extra\":true}",
        "{\"id\":7,\"names\":[\"x\",\"y\"]}",
        "{malformed json",
        "{\"nested\":{\"name\":\"Carol\"},\"other\":1}"
    };
    for (int t = 0; t < 5; ++t) {
        const unsigned char* body = (const unsigned char*)tests[t];
        size_t len = strlen(tests[t]);
        char* res = handle_request(ct, body, len);
        if (res) {
            printf("Test %d => %s\n", t+1, res);
            free(res);
        } else {
            printf("Test %d => \n", t+1);
        }
    }
    return 0;
}