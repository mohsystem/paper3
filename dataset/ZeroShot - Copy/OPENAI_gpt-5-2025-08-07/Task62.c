#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>

#define MAX_LEN 1000000
#define MAX_DEPTH 512

typedef enum { T_OBJECT, T_ARRAY, T_STRING, T_NUMBER, T_BOOLEAN, T_NULL, T_INVALID } JType;

typedef struct {
    const char* s;
    size_t len;
    size_t pos;
    int depth;
} Parser;

static void skip_ws(Parser* p) {
    while (p->pos < p->len) {
        char c = p->s[p->pos];
        if (c==' '||c=='\t'||c=='\r'||c=='\n') p->pos++;
        else break;
    }
}

static int is_hex(char c){
    return (c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F');
}

static int parse_string(Parser* p) {
    if (p->pos >= p->len || p->s[p->pos] != '"') return 0;
    p->pos++;
    while (p->pos < p->len) {
        char c = p->s[p->pos++];
        if (c == '"') return 1;
        if (c == '\\') {
            if (p->pos >= p->len) return 0;
            char e = p->s[p->pos++];
            switch (e) {
                case '"': case '\\': case '/': case 'b':
                case 'f': case 'n': case 'r': case 't':
                    break;
                case 'u':
                    for (int i=0;i<4;i++){
                        if (p->pos >= p->len) return 0;
                        if (!is_hex(p->s[p->pos++])) return 0;
                    }
                    break;
                default:
                    return 0;
            }
        } else {
            if ((unsigned char)c < 0x20) return 0;
        }
    }
    return 0;
}

static int parse_number(Parser* p) {
    size_t start = p->pos;
    if (p->pos < p->len && p->s[p->pos] == '-') p->pos++;
    if (p->pos >= p->len) return 0;
    if (p->s[p->pos] == '0') {
        p->pos++;
    } else if (p->s[p->pos] >= '1' && p->s[p->pos] <= '9') {
        while (p->pos < p->len && p->s[p->pos] >= '0' && p->s[p->pos] <= '9') p->pos++;
    } else return 0;
    if (p->pos < p->len && p->s[p->pos] == '.') {
        p->pos++;
        if (p->pos >= p->len || !(p->s[p->pos] >= '0' && p->s[p->pos] <= '9')) return 0;
        while (p->pos < p->len && p->s[p->pos] >= '0' && p->s[p->pos] <= '9') p->pos++;
    }
    if (p->pos < p->len && (p->s[p->pos] == 'e' || p->s[p->pos] == 'E')) {
        p->pos++;
        if (p->pos < p->len && (p->s[p->pos] == '+' || p->s[p->pos] == '-')) p->pos++;
        if (p->pos >= p->len || !(p->s[p->pos] >= '0' && p->s[p->pos] <= '9')) return 0;
        while (p->pos < p->len && p->s[p->pos] >= '0' && p->s[p->pos] <= '9') p->pos++;
    }
    return p->pos > start;
}

static int match_lit(Parser* p, const char* lit) {
    size_t L = strlen(lit);
    if (p->pos + L > p->len) return 0;
    for (size_t i=0;i<L;i++) if (p->s[p->pos+i] != lit[i]) return 0;
    p->pos += L;
    return 1;
}

static int parse_value(Parser* p, JType* out);

static int parse_object(Parser* p) {
    if (p->depth >= MAX_DEPTH) return 0;
    if (p->pos >= p->len || p->s[p->pos] != '{') return 0;
    p->depth++;
    p->pos++;
    skip_ws(p);
    if (p->pos < p->len && p->s[p->pos] == '}') {
        p->pos++;
        p->depth--;
        return 1;
    }
    while (1) {
        skip_ws(p);
        if (!parse_string(p)) { p->depth--; return 0; }
        skip_ws(p);
        if (p->pos >= p->len || p->s[p->pos] != ':') { p->depth--; return 0; }
        p->pos++;
        JType tmp;
        if (!parse_value(p, &tmp)) { p->depth--; return 0; }
        skip_ws(p);
        if (p->pos < p->len && p->s[p->pos] == ',') {
            p->pos++;
            continue;
        } else if (p->pos < p->len && p->s[p->pos] == '}') {
            p->pos++;
            p->depth--;
            return 1;
        } else {
            p->depth--;
            return 0;
        }
    }
}

static int parse_array(Parser* p) {
    if (p->depth >= MAX_DEPTH) return 0;
    if (p->pos >= p->len || p->s[p->pos] != '[') return 0;
    p->depth++;
    p->pos++;
    skip_ws(p);
    if (p->pos < p->len && p->s[p->pos] == ']') {
        p->pos++;
        p->depth--;
        return 1;
    }
    while (1) {
        JType tmp;
        if (!parse_value(p, &tmp)) { p->depth--; return 0; }
        skip_ws(p);
        if (p->pos < p->len && p->s[p->pos] == ',') {
            p->pos++;
            continue;
        } else if (p->pos < p->len && p->s[p->pos] == ']') {
            p->pos++;
            p->depth--;
            return 1;
        } else {
            p->depth--;
            return 0;
        }
    }
}

static int parse_value(Parser* p, JType* out) {
    skip_ws(p);
    if (p->pos >= p->len) return 0;
    char c = p->s[p->pos];
    if (c == '{') {
        if (!parse_object(p)) return 0;
        *out = T_OBJECT; return 1;
    }
    if (c == '[') {
        if (!parse_array(p)) return 0;
        *out = T_ARRAY; return 1;
    }
    if (c == '"') {
        if (!parse_string(p)) return 0;
        *out = T_STRING; return 1;
    }
    if (c == '-' || (c >= '0' && c <= '9')) {
        if (!parse_number(p)) return 0;
        *out = T_NUMBER; return 1;
    }
    if (match_lit(p, "true")) { *out = T_BOOLEAN; return 1; }
    if (match_lit(p, "false")) { *out = T_BOOLEAN; return 1; }
    if (match_lit(p, "null")) { *out = T_NULL; return 1; }
    return 0;
}

const char* identifyRoot(const char* json) {
    if (json == NULL) return "invalid";
    size_t n = strlen(json);
    if (n > MAX_LEN) return "invalid";
    Parser p;
    p.s = json;
    p.len = n;
    p.pos = 0;
    p.depth = 0;
    skip_ws(&p);
    if (p.pos >= p.len) return "invalid";
    JType t;
    if (!parse_value(&p, &t)) return "invalid";
    skip_ws(&p);
    if (p.pos != p.len) return "invalid";
    switch (t) {
        case T_OBJECT: return "object";
        case T_ARRAY: return "array";
        case T_STRING: return "string";
        case T_NUMBER: return "number";
        case T_BOOLEAN: return "boolean";
        case T_NULL: return "null";
        default: return "invalid";
    }
}

int main(void) {
    const char* tests[5] = {
        "{\"a\":1,\"b\":[true,false,null]}",
        "[1,2,3]",
        "\"hello\"",
        "123.45e-6",
        "{unquoted: 1}"
    };
    for (int i = 0; i < 5; i++) {
        printf("%s\n", identifyRoot(tests[i]));
    }
    return 0;
}