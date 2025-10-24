#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
Chain-of-Through Secure Implementation
1) Understand: Parse JSON structure, return root type and structure.
2) Security: No external libs, strict bounds checks, safe dynamic buffers.
3) Implement: Defensive recursive-descent parser with recursion limit.
4) Review: Validate indices and token flow; handle escapes carefully.
5) Output: "Root=<type>; Structure=<structure>" or invalid.
*/

typedef struct {
    char* data;
    size_t len;
    size_t cap;
} StrBuf;

static int sb_init(StrBuf* b) {
    b->cap = 256;
    b->len = 0;
    b->data = (char*)malloc(b->cap);
    if (!b->data) return 0;
    b->data[0] = '\0';
    return 1;
}
static void sb_free(StrBuf* b) {
    if (b->data) free(b->data);
    b->data = NULL; b->len = b->cap = 0;
}
static int sb_ensure(StrBuf* b, size_t add) {
    if (b->len + add + 1 <= b->cap) return 1;
    size_t ncap = b->cap;
    while (b->len + add + 1 > ncap) {
        ncap *= 2;
        if (ncap < b->cap) return 0; // overflow guard
    }
    char* nd = (char*)realloc(b->data, ncap);
    if (!nd) return 0;
    b->data = nd;
    b->cap = ncap;
    return 1;
}
static int sb_append_cstr(StrBuf* b, const char* s) {
    size_t L = strlen(s);
    if (!sb_ensure(b, L)) return 0;
    memcpy(b->data + b->len, s, L);
    b->len += L;
    b->data[b->len] = '\0';
    return 1;
}
static int sb_append_char(StrBuf* b, char c) {
    if (!sb_ensure(b, 1)) return 0;
    b->data[b->len++] = c;
    b->data[b->len] = '\0';
    return 1;
}

// Parser state
typedef struct {
    const char* s;
    size_t i;
    size_t n;
    int depth;
    int error;
} Parser;

static void skip_ws(Parser* p) {
    while (p->i < p->n) {
        char c = p->s[p->i];
        if (c==' '||c=='\t'||c=='\r'||c=='\n') p->i++;
        else break;
    }
}

static int is_hex(char c) {
    return (c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F');
}

static char* escape_output(const char* in) {
    StrBuf b;
    if (!sb_init(&b)) return NULL;
    for (const char* p = in; *p; ++p) {
        char c = *p;
        if (c == '\\' || c == '"') {
            if (!sb_append_char(&b, '\\')) { sb_free(&b); return NULL; }
            if (!sb_append_char(&b, c)) { sb_free(&b); return NULL; }
        } else if ((unsigned char)c <= 0x1F) {
            if (!sb_append_char(&b, '?')) { sb_free(&b); return NULL; }
        } else {
            if (!sb_append_char(&b, c)) { sb_free(&b); return NULL; }
        }
    }
    return b.data; // caller takes ownership
}

static int match_literal(Parser* p, const char* lit) {
    size_t len = strlen(lit);
    if (p->i + len > p->n) return 0;
    if (strncmp(p->s + p->i, lit, len) == 0) {
        p->i += len;
        return 1;
    }
    return 0;
}

// Forward declarations
static char* parse_value(Parser* p);

static char* parse_string(Parser* p) {
    if (p->i >= p->n || p->s[p->i] != '"') return NULL;
    p->i++;
    StrBuf b;
    if (!sb_init(&b)) return NULL;
    while (p->i < p->n) {
        char c = p->s[p->i++];
        if (c == '"') {
            return b.data; // success
        } else if (c == '\\') {
            if (p->i >= p->n) { sb_free(&b); return NULL; }
            char esc = p->s[p->i++];
            switch (esc) {
                case '"': if (!sb_append_char(&b, '"')) { sb_free(&b); return NULL; } break;
                case '\\': if (!sb_append_char(&b, '\\')) { sb_free(&b); return NULL; } break;
                case '/': if (!sb_append_char(&b, '/')) { sb_free(&b); return NULL; } break;
                case 'b': if (!sb_append_char(&b, '\b')) { sb_free(&b); return NULL; } break;
                case 'f': if (!sb_append_char(&b, '\f')) { sb_free(&b); return NULL; } break;
                case 'n': if (!sb_append_char(&b, '\n')) { sb_free(&b); return NULL; } break;
                case 'r': if (!sb_append_char(&b, '\r')) { sb_free(&b); return NULL; } break;
                case 't': if (!sb_append_char(&b, '\t')) { sb_free(&b); return NULL; } break;
                case 'u': {
                    if (p->i + 4 > p->n) { sb_free(&b); return NULL; }
                    for (int k = 0; k < 4; ++k) {
                        if (!is_hex(p->s[p->i + k])) { sb_free(&b); return NULL; }
                    }
                    p->i += 4;
                    if (!sb_append_char(&b, '?')) { sb_free(&b); return NULL; } // placeholder
                    break;
                }
                default: sb_free(&b); return NULL;
            }
        } else {
            if ((unsigned char)c <= 0x1F) { sb_free(&b); return NULL; }
            if (!sb_append_char(&b, c)) { sb_free(&b); return NULL; }
        }
    }
    sb_free(&b);
    return NULL; // unterminated
}

static int parse_number(Parser* p) {
    size_t start = p->i;
    if (p->i < p->n && p->s[p->i] == '-') p->i++;
    if (p->i >= p->n) return 0;
    if (p->s[p->i] == '0') {
        p->i++;
    } else if (isdigit((unsigned char)p->s[p->i])) {
        p->i++;
        while (p->i < p->n && isdigit((unsigned char)p->s[p->i])) p->i++;
    } else {
        return 0;
    }
    if (p->i < p->n && p->s[p->i] == '.') {
        p->i++;
        if (p->i >= p->n || !isdigit((unsigned char)p->s[p->i])) return 0;
        while (p->i < p->n && isdigit((unsigned char)p->s[p->i])) p->i++;
    }
    if (p->i < p->n && (p->s[p->i] == 'e' || p->s[p->i] == 'E')) {
        p->i++;
        if (p->i < p->n && (p->s[p->i] == '+' || p->s[p->i] == '-')) p->i++;
        if (p->i >= p->n || !isdigit((unsigned char)p->s[p->i])) return 0;
        while (p->i < p->n && isdigit((unsigned char)p->s[p->i])) p->i++;
    }
    return p->i > start;
}

static char* str_dup(const char* s) {
    size_t L = strlen(s);
    char* d = (char*)malloc(L + 1);
    if (!d) return NULL;
    memcpy(d, s, L + 1);
    return d;
}

static int contains_str(char** arr, int count, const char* s) {
    for (int k = 0; k < count; ++k) {
        if (strcmp(arr[k], s) == 0) return 1;
    }
    return 0;
}

static char* parse_array(Parser* p);

static char* parse_object(Parser* p);

static char* parse_value(Parser* p) {
    if (p->depth > 1000) return NULL;
    skip_ws(p);
    if (p->i >= p->n) return NULL;
    char c = p->s[p->i];
    if (c == '{') {
        p->depth++;
        char* r = parse_object(p);
        p->depth--;
        return r;
    }
    if (c == '[') {
        p->depth++;
        char* r = parse_array(p);
        p->depth--;
        return r;
    }
    if (c == '"') {
        char* tmp = parse_string(p);
        if (!tmp) return NULL;
        free(tmp);
        return str_dup("string");
    }
    if (c == '-' || isdigit((unsigned char)c)) {
        if (!parse_number(p)) return NULL;
        return str_dup("number");
    }
    if (match_literal(p, "true")) return str_dup("boolean");
    if (match_literal(p, "false")) return str_dup("boolean");
    if (match_literal(p, "null")) return str_dup("null");
    return NULL;
}

static char* parse_object(Parser* p) {
    if (p->i >= p->n || p->s[p->i] != '{') return NULL;
    p->i++;
    skip_ws(p);
    if (p->i < p->n && p->s[p->i] == '}') {
        p->i++;
        return str_dup("{}");
    }
    // We'll build structure in StrBuf
    StrBuf out;
    if (!sb_init(&out)) return NULL;
    if (!sb_append_char(&out, '{')) { sb_free(&out); return NULL; }
    int first = 1;
    while (1) {
        skip_ws(p);
        if (p->i >= p->n || p->s[p->i] != '"') { sb_free(&out); return NULL; }
        char* key = parse_string(p);
        if (!key) { sb_free(&out); return NULL; }
        skip_ws(p);
        if (p->i >= p->n || p->s[p->i] != ':') { free(key); sb_free(&out); return NULL; }
        p->i++;
        skip_ws(p);
        char* val = parse_value(p);
        if (!val) { free(key); sb_free(&out); return NULL; }
        char* keyEsc = escape_output(key);
        free(key);
        if (!keyEsc) { free(val); sb_free(&out); return NULL; }
        if (!first) {
            if (!sb_append_cstr(&out, ", ")) { free(val); free(keyEsc); sb_free(&out); return NULL; }
        }
        first = 0;
        if (!sb_append_char(&out, '"') || !sb_append_cstr(&out, keyEsc) || !sb_append_cstr(&out, "\": ") || !sb_append_cstr(&out, val)) {
            free(val); free(keyEsc); sb_free(&out); return NULL;
        }
        free(val);
        free(keyEsc);
        skip_ws(p);
        if (p->i >= p->n) { sb_free(&out); return NULL; }
        if (p->s[p->i] == ',') {
            p->i++;
            continue;
        } else if (p->s[p->i] == '}') {
            p->i++;
            break;
        } else {
            sb_free(&out);
            return NULL;
        }
    }
    if (!sb_append_char(&out, '}')) { sb_free(&out); return NULL; }
    return out.data; // caller owns
}

static char* parse_array(Parser* p) {
    if (p->i >= p->n || p->s[p->i] != '[') return NULL;
    p->i++;
    skip_ws(p);
    if (p->i < p->n && p->s[p->i] == ']') {
        p->i++;
        return str_dup("[]");
    }
    // Collect element structures
    int cap = 8, cnt = 0;
    char** elems = (char**)malloc(sizeof(char*) * cap);
    if (!elems) return NULL;
    while (1) {
        char* v = parse_value(p);
        if (!v) {
            for (int k = 0; k < cnt; ++k) free(elems[k]);
            free(elems);
            return NULL;
        }
        if (cnt >= cap) {
            cap *= 2;
            char** ne = (char**)realloc(elems, sizeof(char*) * cap);
            if (!ne) {
                for (int k = 0; k < cnt; ++k) free(elems[k]);
                free(elems);
                free(v);
                return NULL;
            }
            elems = ne;
        }
        elems[cnt++] = v;
        skip_ws(p);
        if (p->i >= p->n) {
            for (int k = 0; k < cnt; ++k) free(elems[k]);
            free(elems);
            return NULL;
        }
        if (p->s[p->i] == ',') {
            p->i++;
            skip_ws(p);
            continue;
        } else if (p->s[p->i] == ']') {
            p->i++;
            break;
        } else {
            for (int k = 0; k < cnt; ++k) free(elems[k]);
            free(elems);
            return NULL;
        }
    }
    // Build unique list preserve order
    int ucap = cnt, ucnt = 0;
    char** uniq = (char**)malloc(sizeof(char*) * (ucap > 1 ? ucap : 1));
    if (!uniq) {
        for (int k = 0; k < cnt; ++k) free(elems[k]);
        free(elems);
        return NULL;
    }
    for (int k = 0; k < cnt; ++k) {
        if (!contains_str(uniq, ucnt, elems[k])) {
            uniq[ucnt++] = elems[k];
        } else {
            free(elems[k]);
        }
    }
    free(elems);
    StrBuf out;
    if (!sb_init(&out)) {
        for (int k = 0; k < ucnt; ++k) free(uniq[k]);
        free(uniq);
        return NULL;
    }
    if (!sb_append_char(&out, '[')) {
        sb_free(&out);
        for (int k = 0; k < ucnt; ++k) free(uniq[k]);
        free(uniq);
        return NULL;
    }
    for (int k = 0; k < ucnt; ++k) {
        if (k > 0) {
            if (!sb_append_cstr(&out, " | ")) { sb_free(&out); goto cleanup_fail; }
        }
        if (!sb_append_cstr(&out, uniq[k])) { sb_free(&out); goto cleanup_fail; }
    }
    if (!sb_append_char(&out, ']')) { sb_free(&out); goto cleanup_fail; }
    for (int k = 0; k < ucnt; ++k) free(uniq[k]);
    free(uniq);
    return out.data;

cleanup_fail:
    for (int k = 0; k < ucnt; ++k) free(uniq[k]);
    free(uniq);
    return NULL;
}

static char* parse_json_structure(const char* json) {
    if (!json) return NULL;
    Parser p;
    p.s = json;
    p.i = 0;
    p.n = strlen(json);
    p.depth = 0;
    p.error = 0;
    skip_ws(&p);
    char* v = parse_value(&p);
    if (!v) return NULL;
    skip_ws(&p);
    if (p.i != p.n) {
        free(v);
        return NULL;
    }
    return v;
}

static char* extract_root_and_structure(const char* json) {
    char* structStr = parse_json_structure(json);
    if (!structStr) {
        char* res = str_dup("Root=invalid; Structure=invalid");
        return res;
    }
    const char* root = "invalid";
    if (structStr[0] == '{') root = "object";
    else if (structStr[0] == '[') root = "array";
    else if (strcmp(structStr, "string") == 0) root = "string";
    else if (strcmp(structStr, "number") == 0) root = "number";
    else if (strcmp(structStr, "boolean") == 0) root = "boolean";
    else if (strcmp(structStr, "null") == 0) root = "null";
    StrBuf out;
    if (!sb_init(&out)) { free(structStr); return NULL; }
    if (!sb_append_cstr(&out, "Root=") ||
        !sb_append_cstr(&out, root) ||
        !sb_append_cstr(&out, "; Structure=") ||
        !sb_append_cstr(&out, structStr)) {
        sb_free(&out);
        free(structStr);
        return NULL;
    }
    free(structStr);
    return out.data;
}

int main(void) {
    const char* tests[5] = {
        "{\"a\":1,\"b\":{\"c\":[1,2,3]},\"d\":[{\"e\":\"x\"},{\"e\":\"y\",\"f\":true}]}",
        "[1,2,{\"a\":[null,false]}]",
        "\"hello\"",
        "123.45e-6",
        "{unclosed"
    };
    for (int t = 0; t < 5; ++t) {
        char* res = extract_root_and_structure(tests[t]);
        if (res) {
            printf("%s\n", res);
            free(res);
        } else {
            printf("Root=invalid; Structure=invalid\n");
        }
    }
    return 0;
}