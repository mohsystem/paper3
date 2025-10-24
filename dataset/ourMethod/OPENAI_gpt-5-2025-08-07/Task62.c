#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef enum {
    JT_OBJECT,
    JT_ARRAY,
    JT_STRING,
    JT_NUMBER,
    JT_BOOLEAN,
    JT_NULL,
    JT_INVALID
} JSONType;

static const char* json_type_name(JSONType t) {
    switch (t) {
        case JT_OBJECT: return "object";
        case JT_ARRAY: return "array";
        case JT_STRING: return "string";
        case JT_NUMBER: return "number";
        case JT_BOOLEAN: return "boolean";
        case JT_NULL: return "null";
        default: return "invalid";
    }
}

typedef struct {
    int ok;
    JSONType rootType;
    char* rootTypeName;
    char* structure;
    char* error;
} ParseResultC;

typedef struct {
    char* buf;
    size_t len;
    size_t cap;
    size_t max;
    int error;
} StrBuilder;

static void sb_init(StrBuilder* sb, size_t initial, size_t maxcap) {
    sb->buf = (char*)malloc(initial);
    if (sb->buf == NULL) {
        sb->len = sb->cap = 0;
        sb->max = maxcap;
        sb->error = 1;
        return;
    }
    sb->len = 0;
    sb->cap = initial;
    sb->max = maxcap;
    sb->error = 0;
    sb->buf[0] = '\0';
}

static void sb_free(StrBuilder* sb) {
    if (sb->buf) {
        free(sb->buf);
        sb->buf = NULL;
    }
    sb->len = sb->cap = sb->max = 0;
    sb->error = 0;
}

static int sb_ensure(StrBuilder* sb, size_t add) {
    if (sb->error) return 0;
    if (sb->len + add + 1 > sb->max) { sb->error = 1; return 0; }
    if (sb->len + add + 1 <= sb->cap) return 1;
    size_t newcap = sb->cap ? sb->cap : 64;
    while (newcap < sb->len + add + 1) {
        size_t prev = newcap;
        newcap *= 2;
        if (newcap < prev) { sb->error = 1; return 0; } // overflow
        if (newcap > sb->max) { newcap = sb->max; break; }
    }
    if (newcap < sb->len + add + 1) { sb->error = 1; return 0; }
    char* nb = (char*)realloc(sb->buf, newcap);
    if (!nb) { sb->error = 1; return 0; }
    sb->buf = nb;
    sb->cap = newcap;
    return 1;
}

static int sb_append_c(StrBuilder* sb, char c) {
    if (!sb_ensure(sb, 1)) return 0;
    sb->buf[sb->len++] = c;
    sb->buf[sb->len] = '\0';
    return 1;
}

static int sb_append_s(StrBuilder* sb, const char* s) {
    size_t L = strlen(s);
    if (!sb_ensure(sb, L)) return 0;
    memcpy(sb->buf + sb->len, s, L);
    sb->len += L;
    sb->buf[sb->len] = '\0';
    return 1;
}

typedef struct {
    const char* s;
    size_t i;
    size_t n;
    int depth;
    int max_depth;
    size_t max_input;
    StrBuilder out;
    char* err;
    size_t max_key_preview;
    size_t max_array_preview;
} ParserC;

static char* str_dup_c(const char* s) {
    size_t L = strlen(s);
    char* p = (char*)malloc(L + 1);
    if (!p) return NULL;
    memcpy(p, s, L + 1);
    return p;
}

static void set_error(ParserC* p, const char* msg) {
    if (!p->err) p->err = str_dup_c(msg);
}

static void skip_ws(ParserC* p) {
    while (p->i < p->n) {
        char c = p->s[p->i];
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') p->i++;
        else break;
    }
}

static int match_lit(ParserC* p, const char* lit) {
    size_t L = strlen(lit);
    if (p->i + L <= p->n && strncmp(p->s + p->i, lit, L) == 0) {
        p->i += L;
        return 1;
    }
    return 0;
}

static int parse_value(ParserC* p, JSONType* t);

static int parse_string_content(ParserC* p, StrBuilder* storeOpt) {
    if (p->i >= p->n || p->s[p->i] != '"') return 0;
    p->i++;
    while (p->i < p->n) {
        char c = p->s[p->i++];
        if (c == '"') {
            return 1;
        }
        if ((unsigned char)c <= 0x1F) { set_error(p, "Unescaped control char in string"); return 0; }
        if (c == '\\') {
            if (p->i >= p->n) { set_error(p, "Invalid escape"); return 0; }
            char e = p->s[p->i++];
            switch (e) {
                case '"': case '\\': case '/': case 'b': case 'f': case 'n': case 'r': case 't':
                    if (storeOpt && storeOpt->len < storeOpt->max) {
                        // For preview we can approximate escapes, not critical
                        sb_append_c(storeOpt, (e == 'b') ? '\b' :
                                              (e == 'f') ? '\f' :
                                              (e == 'n') ? '\n' :
                                              (e == 'r') ? '\r' :
                                              (e == 't') ? '\t' :
                                              (e == '/') ? '/' : e);
                    }
                    break;
                case 'u': {
                    if (p->i + 4 > p->n) { set_error(p, "Invalid unicode escape"); return 0; }
                    for (int k = 0; k < 4; ++k) {
                        char h = p->s[p->i + k];
                        int hex = (h >= '0' && h <= '9') || (h >= 'a' && h <= 'f') || (h >= 'A' && h <= 'F');
                        if (!hex) { set_error(p, "Invalid unicode escape"); return 0; }
                    }
                    // Append placeholder to preview
                    if (storeOpt && storeOpt->len + 6 < storeOpt->max) {
                        sb_append_s(storeOpt, "\\u");
                        for (int k = 0; k < 4; ++k) sb_append_c(storeOpt, p->s[p->i + k]);
                    }
                    p->i += 4;
                    break;
                }
                default:
                    set_error(p, "Invalid escape");
                    return 0;
            }
        } else {
            if (storeOpt && storeOpt->len < storeOpt->max) {
                sb_append_c(storeOpt, c);
            }
        }
    }
    set_error(p, "Unterminated string");
    return 0;
}

static int append_token(ParserC* p, const char* tok) {
    return sb_append_s(&p->out, tok);
}

static int parse_string(ParserC* p) {
    return parse_string_content(p, NULL);
}

static int parse_number(ParserC* p) {
    size_t i = p->i;
    if (i < p->n && p->s[i] == '-') i++;
    if (i >= p->n) { set_error(p, "Invalid number"); return 0; }
    if (p->s[i] == '0') {
        i++;
    } else if (isdigit((unsigned char)p->s[i])) {
        while (i < p->n && isdigit((unsigned char)p->s[i])) i++;
    } else {
        set_error(p, "Invalid number");
        return 0;
    }
    if (i < p->n && p->s[i] == '.') {
        i++;
        if (i >= p->n || !isdigit((unsigned char)p->s[i])) { set_error(p, "Invalid fraction"); return 0; }
        while (i < p->n && isdigit((unsigned char)p->s[i])) i++;
    }
    if (i < p->n && (p->s[i] == 'e' || p->s[i] == 'E')) {
        i++;
        if (i < p->n && (p->s[i] == '+' || p->s[i] == '-')) i++;
        if (i >= p->n || !isdigit((unsigned char)p->s[i])) { set_error(p, "Invalid exponent"); return 0; }
        while (i < p->n && isdigit((unsigned char)p->s[i])) i++;
    }
    p->i = i;
    return 1;
}

static int parse_object(ParserC* p);
static int parse_array(ParserC* p);

static int parse_value(ParserC* p, JSONType* t) {
    if (++p->depth > p->max_depth) { set_error(p, "Maximum nesting depth exceeded"); return 0; }
    skip_ws(p);
    if (p->i >= p->n) { set_error(p, "Unexpected end of input"); p->depth--; return 0; }
    char c = p->s[p->i];
    int ok = 0;
    if (c == '{') {
        ok = parse_object(p);
        *t = JT_OBJECT;
    } else if (c == '[') {
        ok = parse_array(p);
        *t = JT_ARRAY;
    } else if (c == '"') {
        ok = parse_string(p);
        *t = JT_STRING;
        if (ok) ok = append_token(p, "string");
    } else if (c == '-' || (c >= '0' && c <= '9')) {
        ok = parse_number(p);
        *t = JT_NUMBER;
        if (ok) ok = append_token(p, "number");
    } else if (match_lit(p, "true")) {
        *t = JT_BOOLEAN;
        ok = append_token(p, "boolean");
    } else if (match_lit(p, "false")) {
        *t = JT_BOOLEAN;
        ok = append_token(p, "boolean");
    } else if (match_lit(p, "null")) {
        *t = JT_NULL;
        ok = append_token(p, "null");
    } else {
        set_error(p, "Invalid value");
        ok = 0;
    }
    p->depth--;
    return ok;
}

static int append_key_preview(ParserC* p, const char* key, size_t key_len) {
    // add quotes and truncated content
    if (!sb_append_c(&p->out, '"')) return 0;
    size_t limit = p->max_key_preview;
    size_t to_write = key_len > limit ? limit : key_len;
    if (to_write) {
        if (!sb_ensure(&p->out, to_write)) return 0;
        memcpy(p->out.buf + p->out.len, key, to_write);
        p->out.len += to_write;
        p->out.buf[p->out.len] = '\0';
    }
    if (key_len > limit) {
        if (!sb_append_s(&p->out, "...")) return 0;
    }
    if (!sb_append_c(&p->out, '"')) return 0;
    return 1;
}

static int parse_object(ParserC* p) {
    if (p->s[p->i] != '{') { set_error(p, "Expected '{'"); return 0; }
    p->i++;
    skip_ws(p);
    if (!sb_append_c(&p->out, '{')) return 0;
    if (p->i < p->n && p->s[p->i] == '}') {
        p->i++;
        if (!sb_append_c(&p->out, '}')) return 0;
        return 1;
    }
    int first = 1;
    while (p->i < p->n) {
        skip_ws(p);
        if (p->i >= p->n || p->s[p->i] != '"') { set_error(p, "Expected object key string"); return 0; }
        // capture key preview
        StrBuilder keyPrev;
        sb_init(&keyPrev, 64, p->max_key_preview + 16);
        if (keyPrev.error) { set_error(p, "Memory allocation failed"); return 0; }
        if (!parse_string_content(p, &keyPrev)) { sb_free(&keyPrev); return 0; }

        skip_ws(p);
        if (p->i >= p->n || p->s[p->i] != ':') { set_error(p, "Expected ':' after key"); sb_free(&keyPrev); return 0; }
        p->i++;
        if (!first) { if (!sb_append_c(&p->out, ',')) { sb_free(&keyPrev); return 0; } }
        first = 0;
        if (!append_key_preview(p, keyPrev.buf ? keyPrev.buf : "", keyPrev.len)) { sb_free(&keyPrev); return 0; }
        if (!sb_append_c(&p->out, ':')) { sb_free(&keyPrev); return 0; }
        sb_free(&keyPrev);

        skip_ws(p);
        JSONType inner = JT_INVALID;
        if (!parse_value(p, &inner)) return 0;

        skip_ws(p);
        if (p->i < p->n && p->s[p->i] == ',') {
            p->i++;
            skip_ws(p);
            continue;
        } else if (p->i < p->n && p->s[p->i] == '}') {
            p->i++;
            if (!sb_append_c(&p->out, '}')) return 0;
            return 1;
        } else {
            set_error(p, "Expected ',' or '}' in object");
            return 0;
        }
    }
    set_error(p, "Unterminated object");
    return 0;
}

static int parse_array(ParserC* p) {
    if (p->s[p->i] != '[') { set_error(p, "Expected '['"); return 0; }
    p->i++;
    skip_ws(p);
    if (!sb_append_c(&p->out, '[')) return 0;
    if (p->i < p->n && p->s[p->i] == ']') {
        p->i++;
        if (!sb_append_c(&p->out, ']')) return 0;
        return 1;
    }
    int first = 1;
    size_t count = 0;
    while (p->i < p->n) {
        if (!first) { if (!sb_append_c(&p->out, ',')) return 0; }
        first = 0;
        skip_ws(p);
        JSONType inner = JT_INVALID;
        if (count < p->max_array_preview) {
            if (!parse_value(p, &inner)) return 0;
        } else {
            // parse silently: temporarily stash out buffer length and restore
            size_t saved_len = p->out.len;
            // Parse and then revert appended structure
            if (!parse_value(p, &inner)) return 0;
            p->out.len = saved_len;
            p->out.buf[saved_len] = '\0';
        }
        count++;
        skip_ws(p);
        if (p->i < p->n && p->s[p->i] == ',') {
            p->i++;
            skip_ws(p);
            continue;
        } else if (p->i < p->n && p->s[p->i] == ']') {
            p->i++;
            if (count > p->max_array_preview) {
                if (!sb_append_s(&p->out, "...")) return 0;
            }
            if (!sb_append_c(&p->out, ']')) return 0;
            return 1;
        } else {
            set_error(p, "Expected ',' or ']' in array");
            return 0;
        }
    }
    set_error(p, "Unterminated array");
    return 0;
}

ParseResultC parse_json_structure(const char* input) {
    ParseResultC res;
    res.ok = 0;
    res.rootType = JT_INVALID;
    res.rootTypeName = str_dup_c("invalid");
    res.structure = str_dup_c("");
    res.error = NULL;

    if (input == NULL) {
        res.error = str_dup_c("Null input");
        return res;
    }
    size_t len = strlen(input);
    if (len > 1000000u) {
        res.error = str_dup_c("Input too large");
        return res;
    }

    ParserC p;
    p.s = input;
    p.i = 0;
    p.n = len;
    p.depth = 0;
    p.max_depth = 256;
    p.max_input = 1000000u;
    p.err = NULL;
    p.max_key_preview = 64u;
    p.max_array_preview = 20u;
    sb_init(&p.out, 256, 200000u);
    if (p.out.error) {
        res.error = str_dup_c("Memory allocation failed");
        sb_free(&p.out);
        return res;
    }

    skip_ws(&p);
    if (p.i >= p.n) {
        res.error = str_dup_c("Empty input");
        sb_free(&p.out);
        return res;
    }
    JSONType root = JT_INVALID;
    if (!parse_value(&p, &root)) {
        if (!p.err) p.err = str_dup_c("Parse error");
        res.error = p.err;
        sb_free(&p.out);
        return res;
    }
    skip_ws(&p);
    if (p.i != p.n) {
        res.error = str_dup_c("Trailing characters after JSON value");
        sb_free(&p.out);
        if (p.err) free(p.err);
        return res;
    }
    if (p.out.error) {
        res.error = str_dup_c("Output structure too large");
        sb_free(&p.out);
        if (p.err) free(p.err);
        return res;
    }

    res.ok = 1;
    res.rootType = root;
    if (res.rootTypeName) free(res.rootTypeName);
    res.rootTypeName = str_dup_c(json_type_name(root));
    if (res.structure) free(res.structure);
    res.structure = str_dup_c(p.out.buf ? p.out.buf : "");
    res.error = NULL;

    sb_free(&p.out);
    if (p.err) free(p.err);
    return res;
}

static void free_parse_result(ParseResultC* r) {
    if (!r) return;
    if (r->rootTypeName) { free(r->rootTypeName); r->rootTypeName = NULL; }
    if (r->structure) { free(r->structure); r->structure = NULL; }
    if (r->error) { free(r->error); r->error = NULL; }
}

static void run_test(const char* json) {
    ParseResultC r = parse_json_structure(json);
    printf("Input: %s\n", json);
    if (r.ok) {
        printf("OK Root=%s\n", r.rootTypeName ? r.rootTypeName : "invalid");
        printf("Structure: %s\n", r.structure ? r.structure : "");
    } else {
        printf("ERROR: %s\n", r.error ? r.error : "Unknown error");
    }
    printf("----\n");
    free_parse_result(&r);
}

int main(void) {
    const char* tests[5] = {
        "{\"a\":1,\"b\":[true,null,{\"c\":\"x\"}]}",
        "[1, 2, 3]",
        "\"hello\"",
        "-12.34e+10",
        "{unquoted:1}"
    };
    for (int k = 0; k < 5; ++k) run_test(tests[k]);
    // extra
    run_test("null");
    return 0;
}