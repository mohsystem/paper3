/*
Secure JSON Fetcher and Parser in C
- Fetches a JSON document over HTTPS from a URL provided on the command line or from built-in tests.
- Enforces TLS 1.2+, certificate, and hostname verification using libcurl.
- Limits downloads to a configurable maximum size.
- Parses JSON using an embedded JSMN parser (MIT licensed) and converts to a simple in-memory tree.
- Includes five test URLs when run without arguments.

Build (example):
  cc -Wall -Wextra -O2 -o task64 task64.c -lcurl

Run:
  ./task64 https://example.com/data.json
  or just: ./task64
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

/*
JSMN - Embedded minimalistic JSON parser
https://github.com/zserge/jsmn
MIT License
Only the necessary parts are embedded here in a single compilation unit.
*/

typedef enum {
    JSMN_UNDEFINED = 0,
    JSMN_OBJECT = 1,
    JSMN_ARRAY = 2,
    JSMN_STRING = 3,
    JSMN_PRIMITIVE = 4
} jsmntype_t;

enum jsmnerr {
    JSMN_ERROR_NOMEM = -1,
    JSMN_ERROR_INVAL = -2,
    JSMN_ERROR_PART = -3
};

typedef struct jsmntok {
    jsmntype_t type;
    int start;
    int end;
    int size;
#ifdef JSMN_PARENT_LINKS
    int parent;
#endif
} jsmntok_t;

typedef struct jsmn_parser {
    unsigned int pos;
    unsigned int toknext;
    int toksuper;
} jsmn_parser;

static void jsmn_init(jsmn_parser *parser) {
    parser->pos = 0U;
    parser->toknext = 0U;
    parser->toksuper = -1;
}

static jsmntok_t *jsmn_alloc_token(jsmn_parser *parser, jsmntok_t *tokens, size_t num_tokens) {
    if (parser->toknext >= num_tokens) {
        return NULL;
    }
    jsmntok_t *tok = &tokens[parser->toknext++];
    tok->start = tok->end = -1;
    tok->size = 0;
#ifdef JSMN_PARENT_LINKS
    tok->parent = -1;
#endif
    return tok;
}

static void jsmn_fill_token(jsmntok_t *token, jsmntype_t type, int start, int end) {
    token->type = type;
    token->start = start;
    token->end = end;
    token->size = 0;
}

static int jsmn_parse_primitive(jsmn_parser *parser, const char *js, size_t len,
                                jsmntok_t *tokens, size_t num_tokens) {
    int start = (int)parser->pos;

    for (; parser->pos < len; parser->pos++) {
        char c = js[parser->pos];
        if (c == '\t' || c == '\r' || c == '\n' || c == ' ' ||
            c == ','  || c == ']'  || c == '}') {
            break;
        }
        if ((unsigned char)c < 32 || c == ':') {
            return JSMN_ERROR_INVAL;
        }
    }

    if (tokens == NULL) {
        parser->pos--;
        return 0;
    }
    jsmntok_t *token = jsmn_alloc_token(parser, tokens, num_tokens);
    if (token == NULL) {
        parser->pos = (unsigned int)start;
        return JSMN_ERROR_NOMEM;
    }
    jsmn_fill_token(token, JSMN_PRIMITIVE, start, (int)parser->pos);
#ifdef JSMN_PARENT_LINKS
    token->parent = parser->toksuper;
#endif
    parser->pos--;
    return 0;
}

static int jsmn_parse_string(jsmn_parser *parser, const char *js, size_t len,
                             jsmntok_t *tokens, size_t num_tokens) {
    int start = (int)parser->pos;

    parser->pos++;
    for (; parser->pos < len; parser->pos++) {
        char c = js[parser->pos];

        if (c == '\"') {
            if (tokens == NULL) {
                return 0;
            }
            jsmntok_t *token = jsmn_alloc_token(parser, tokens, num_tokens);
            if (token == NULL) {
                parser->pos = (unsigned int)start;
                return JSMN_ERROR_NOMEM;
            }
            jsmn_fill_token(token, JSMN_STRING, start + 1, (int)parser->pos);
#ifdef JSMN_PARENT_LINKS
            token->parent = parser->toksuper;
#endif
            return 0;
        }

        if (c == '\\') {
            parser->pos++;
            if (parser->pos == len) {
                return JSMN_ERROR_PART;
            }
            char esc = js[parser->pos];
            switch (esc) {
                case '\"': case '/' : case '\\': case 'b':
                case 'f' : case 'r' : case 'n' : case 't':
                    break;
                case 'u':
                    for (int i = 0; i < 4; i++) {
                        parser->pos++;
                        if (parser->pos == len) {
                            return JSMN_ERROR_PART;
                        }
                        char hc = js[parser->pos];
                        if (!((hc >= '0' && hc <= '9') ||
                              (hc >= 'A' && hc <= 'F') ||
                              (hc >= 'a' && hc <= 'f'))) {
                            return JSMN_ERROR_INVAL;
                        }
                    }
                    break;
                default:
                    return JSMN_ERROR_INVAL;
            }
        }
    }
    return JSMN_ERROR_PART;
}

static int jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
                      jsmntok_t *tokens, unsigned int num_tokens) {
    int r;
    int i;
    jsmntok_t *token;
    int count = (int)parser->toknext;

    for (; parser->pos < len; parser->pos++) {
        char c = js[parser->pos];
        jsmntype_t type;

        switch (c) {
            case '{':
            case '[':
                count++;
                if (tokens == NULL) {
                    break;
                }
                token = jsmn_alloc_token(parser, tokens, num_tokens);
                if (token == NULL) {
                    return JSMN_ERROR_NOMEM;
                }
                if (c == '{') {
                    type = JSMN_OBJECT;
                } else {
                    type = JSMN_ARRAY;
                }
                jsmn_fill_token(token, type, (int)parser->pos, -1);
#ifdef JSMN_PARENT_LINKS
                token->parent = parser->toksuper;
#endif
                parser->toksuper = (int)(parser->toknext - 1);
                break;

            case '}':
            case ']':
                if (tokens == NULL) {
                    break;
                }
                type = (c == '}') ? JSMN_OBJECT : JSMN_ARRAY;
                for (i = (int)parser->toknext - 1; i >= 0; i--) {
                    token = &tokens[i];
                    if (token->start != -1 && token->end == -1) {
                        if (token->type != type) {
                            return JSMN_ERROR_INVAL;
                        }
                        token->end = (int)parser->pos + 1;
                        parser->toksuper =
#ifdef JSMN_PARENT_LINKS
                            token->parent;
#else
                            -1;
                        for (i = (int)parser->toknext - 1; i >= 0; i--) {
                            if (tokens[i].start != -1 && tokens[i].end == -1) {
                                parser->toksuper = i;
                                break;
                            }
                        }
#endif
                        break;
                    }
                }
                if (i == -1) return JSMN_ERROR_INVAL;
                break;

            case '\"':
                r = jsmn_parse_string(parser, js, len, tokens, num_tokens);
                if (r < 0) return r;
                count++;
                if (parser->toksuper != -1 && tokens != NULL) {
                    tokens[(size_t)parser->toksuper].size++;
                }
                break;

            case '\t': case '\r': case '\n': case ' ':
                break;

            case ':':
                break;

            case ',':
                if (tokens != NULL && parser->toksuper != -1 &&
                    tokens[(size_t)parser->toksuper].type != JSMN_ARRAY &&
                    tokens[(size_t)parser->toksuper].type != JSMN_OBJECT) {
                    for (i = (int)parser->toknext - 1; i >= 0; i--) {
                        if (tokens[(size_t)i].type == JSMN_ARRAY || tokens[(size_t)i].type == JSMN_OBJECT) {
                            if (tokens[(size_t)i].start != -1 && tokens[(size_t)i].end == -1) {
                                parser->toksuper = i;
                                break;
                            }
                        }
                    }
                }
                break;

            default:
                r = jsmn_parse_primitive(parser, js, len, tokens, num_tokens);
                if (r < 0) return r;
                count++;
                if (parser->toksuper != -1 && tokens != NULL) {
                    tokens[(size_t)parser->toksuper].size++;
                }
                break;
        }
    }

    for (i = (int)parser->toknext - 1; i >= 0; i--) {
        if (tokens[(size_t)i].start != -1 && tokens[(size_t)i].end == -1) {
            return JSMN_ERROR_PART;
        }
    }
    return count;
}

/* Simple JSON DOM structure */

typedef enum {
    JN_OBJECT,
    JN_ARRAY,
    JN_STRING,
    JN_NUMBER,
    JN_BOOL,
    JN_NULL
} JsonType;

struct JsonNode;

typedef struct {
    char *key;
    struct JsonNode *value;
} JsonPair;

typedef struct JsonNode {
    JsonType type;
    union {
        struct {
            JsonPair *pairs;
            size_t count;
        } object;
        struct {
            struct JsonNode **items;
            size_t count;
        } array;
        char *string;
        double number;
        int boolean;
    } u;
} JsonNode;

/* Memory utilities */

static void secure_free(void *p, size_t sz) {
#if defined(__STDC_LIB_EXT1__)
    if (p && sz > 0) {
        memset_s(p, sz, 0, sz);
        free(p);
    } else if (p) {
        free(p);
    }
#else
    if (p && sz > 0) {
        volatile unsigned char *vp = (volatile unsigned char *)p;
        while (sz--) *vp++ = 0;
    }
    if (p) free(p);
#endif
}

static char *safe_strndup(const char *s, size_t n) {
    char *out = (char *)malloc(n + 1);
    if (!out) return NULL;
    if (n > 0) memcpy(out, s, n);
    out[n] = '\0';
    return out;
}

static int is_json_true(const char *s, size_t n) {
    return (n == 4 && s[0]=='t' && s[1]=='r' && s[2]=='u' && s[3]=='e');
}
static int is_json_false(const char *s, size_t n) {
    return (n == 5 && s[0]=='f' && s[1]=='a' && s[2]=='l' && s[3]=='s' && s[4]=='e');
}
static int is_json_null(const char *s, size_t n) {
    return (n == 4 && s[0]=='n' && s[1]=='u' && s[2]=='l' && s[3]=='l');
}

static int parse_double_strict(const char *s, size_t n, double *out) {
    if (!s || n == 0 || !out) return 0;
    char *buf = (char *)malloc(n + 1);
    if (!buf) return 0;
    memcpy(buf, s, n);
    buf[n] = '\0';

    char *endp = NULL;
    errno = 0;
    double val = strtod(buf, &endp);
    int ok = 0;
    if (errno == 0 && endp && *endp == '\0') {
        ok = 1;
        *out = val;
    }
    secure_free(buf, n + 1);
    return ok;
}

/* Convert JSMN tokens into JsonNode tree */

typedef struct {
    const char *json;
    const jsmntok_t *toks;
    size_t tokc;
    size_t idx;
    int depth;
    int depth_limit;
    size_t node_cap_limit;
} TokCursor;

static void free_json(JsonNode *n) {
    if (!n) return;
    switch (n->type) {
        case JN_STRING:
            secure_free(n->u.string, n->u.string ? strlen(n->u.string) : 0);
            break;
        case JN_NUMBER:
            break;
        case JN_BOOL:
        case JN_NULL:
            break;
        case JN_ARRAY:
            if (n->u.array.items) {
                for (size_t i = 0; i < n->u.array.count; i++) {
                    free_json(n->u.array.items[i]);
                }
                free(n->u.array.items);
            }
            break;
        case JN_OBJECT:
            if (n->u.object.pairs) {
                for (size_t i = 0; i < n->u.object.count; i++) {
                    secure_free(n->u.object.pairs[i].key,
                                n->u.object.pairs[i].key ? strlen(n->u.object.pairs[i].key) : 0);
                    free_json(n->u.object.pairs[i].value);
                }
                free(n->u.object.pairs);
            }
            break;
    }
    free(n);
}

static JsonNode *build_node(TokCursor *cur, char *errbuf, size_t errlen);

static JsonNode *build_string(TokCursor *cur, const jsmntok_t *tok) {
    size_t len = (tok->end > tok->start && tok->start >= 0) ? (size_t)(tok->end - tok->start) : 0;
    const char *start = cur->json + tok->start;
    char *s = safe_strndup(start, len);
    if (!s) return NULL;
    JsonNode *n = (JsonNode *)calloc(1, sizeof(JsonNode));
    if (!n) {
        secure_free(s, len + 1);
        return NULL;
    }
    n->type = JN_STRING;
    n->u.string = s;
    return n;
}

static JsonNode *build_primitive(TokCursor *cur, const jsmntok_t *tok) {
    size_t len = (tok->end > tok->start && tok->start >= 0) ? (size_t)(tok->end - tok->start) : 0;
    const char *start = cur->json + tok->start;

    if (is_json_true(start, len)) {
        JsonNode *n = (JsonNode *)calloc(1, sizeof(JsonNode));
        if (!n) return NULL;
        n->type = JN_BOOL;
        n->u.boolean = 1;
        return n;
    }
    if (is_json_false(start, len)) {
        JsonNode *n = (JsonNode *)calloc(1, sizeof(JsonNode));
        if (!n) return NULL;
        n->type = JN_BOOL;
        n->u.boolean = 0;
        return n;
    }
    if (is_json_null(start, len)) {
        JsonNode *n = (JsonNode *)calloc(1, sizeof(JsonNode));
        if (!n) return NULL;
        n->type = JN_NULL;
        return n;
    }
    double d = 0.0;
    if (!parse_double_strict(start, len, &d)) {
        return NULL;
    }
    JsonNode *n = (JsonNode *)calloc(1, sizeof(JsonNode));
    if (!n) return NULL;
    n->type = JN_NUMBER;
    n->u.number = d;
    return n;
}

static JsonNode *build_array(TokCursor *cur, const jsmntok_t *tok, char *errbuf, size_t errlen) {
    if (cur->depth >= cur->depth_limit) {
        if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
        return NULL;
    }
    JsonNode *n = (JsonNode *)calloc(1, sizeof(JsonNode));
    if (!n) return NULL;
    n->type = JN_ARRAY;
    if (tok->size < 0 || (size_t)tok->size > cur->node_cap_limit) {
        free(n);
        return NULL;
    }
    size_t cnt = (size_t)tok->size;
    n->u.array.count = cnt;
    if (cnt > 0) {
        n->u.array.items = (JsonNode **)calloc(cnt, sizeof(JsonNode *));
        if (!n->u.array.items) {
            free(n);
            return NULL;
        }
        cur->idx++; /* move to first child token */
        cur->depth++;
        for (size_t i = 0; i < cnt; i++) {
            if (cur->idx >= cur->tokc) { cur->depth--; free_json(n); return NULL; }
            JsonNode *child = build_node(cur, errbuf, errlen);
            if (!child) { cur->depth--; free_json(n); return NULL; }
            n->u.array.items[i] = child;
        }
        cur->depth--;
        return n;
    } else {
        cur->idx++; /* no children, still consume this array token */
        return n;
    }
}

static JsonNode *build_object(TokCursor *cur, const jsmntok_t *tok, char *errbuf, size_t errlen) {
    if (cur->depth >= cur->depth_limit) {
        if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
        return NULL;
    }
    JsonNode *n = (JsonNode *)calloc(1, sizeof(JsonNode));
    if (!n) return NULL;
    n->type = JN_OBJECT;
    if (tok->size < 0 || (size_t)tok->size > cur->node_cap_limit) {
        free(n);
        return NULL;
    }
    size_t paircnt = (size_t)tok->size;
    n->u.object.count = paircnt;
    if (paircnt > 0) {
        n->u.object.pairs = (JsonPair *)calloc(paircnt, sizeof(JsonPair));
        if (!n->u.object.pairs) { free(n); return NULL; }
        cur->idx++; /* move to first key token */
        cur->depth++;
        for (size_t i = 0; i < paircnt; i++) {
            if (cur->idx >= cur->tokc) { cur->depth--; free_json(n); return NULL; }
            const jsmntok_t *ktok = &cur->toks[cur->idx];
            if (ktok->type != JSMN_STRING) { cur->depth--; free_json(n); return NULL; }
            JsonNode *keynode = build_node(cur, errbuf, errlen);
            if (!keynode || keynode->type != JN_STRING) { if (keynode) free_json(keynode); cur->depth--; free_json(n); return NULL; }
            n->u.object.pairs[i].key = keynode->u.string;
            keynode->u.string = NULL;
            free_json(keynode);

            if (cur->idx >= cur->tokc) { cur->depth--; free_json(n); return NULL; }
            JsonNode *val = build_node(cur, errbuf, errlen);
            if (!val) { cur->depth--; free_json(n); return NULL; }
            n->u.object.pairs[i].value = val;
        }
        cur->depth--;
        return n;
    } else {
        cur->idx++; /* empty object token consumed */
        return n;
    }
}

static JsonNode *build_node(TokCursor *cur, char *errbuf, size_t errlen) {
    if (cur->idx >= cur->tokc) return NULL;
    const jsmntok_t *tok = &cur->toks[cur->idx];
    switch (tok->type) {
        case JSMN_STRING: {
            JsonNode *s = build_string(cur, tok);
            if (!s) return NULL;
            cur->idx++;
            return s;
        }
        case JSMN_PRIMITIVE: {
            JsonNode *p = build_primitive(cur, tok);
            if (!p) return NULL;
            cur->idx++;
            return p;
        }
        case JSMN_ARRAY:
            return build_array(cur, tok, errbuf, errlen);
        case JSMN_OBJECT:
            return build_object(cur, tok, errbuf, errlen);
        default:
            if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
            return NULL;
    }
}

/* Minimal pretty printer with bounds */

static void fputc_bound(int c, FILE *out, size_t *count, size_t max) {
    if (*count < max) {
        fputc(c, out);
        (*count)++;
    }
}

static void fputs_bound(const char *s, FILE *out, size_t *count, size_t max) {
    while (*s && *count < max) {
        fputc(*s++, out);
        (*count)++;
    }
}

static void print_escaped_str_bound(const char *s, FILE *out, size_t *count, size_t max) {
    fputc_bound('"', out, count, max);
    for (; *s && *count < max; s++) {
        unsigned char c = (unsigned char)*s;
        switch (c) {
            case '\\': fputs_bound("\\\\", out, count, max); break;
            case '"':  fputs_bound("\\\"", out, count, max); break;
            case '\b': fputs_bound("\\b", out, count, max); break;
            case '\f': fputs_bound("\\f", out, count, max); break;
            case '\n': fputs_bound("\\n", out, count, max); break;
            case '\r': fputs_bound("\\r", out, count, max); break;
            case '\t': fputs_bound("\\t", out, count, max); break;
            default:
                if (c < 0x20) {
                    char buf[7];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    fputs_bound(buf, out, count, max);
                } else {
                    fputc_bound(c, out, count, max);
                }
        }
    }
    fputc_bound('"', out, count, max);
}

static void print_indent(int n, FILE *out, size_t *count, size_t max) {
    for (int i = 0; i < n; i++) fputc_bound(' ', out, count, max);
}

static void json_print_bound(const JsonNode *n, int indent, int depth_limit, FILE *out, size_t *count, size_t max) {
    if (!n || depth_limit <= 0) { fputs_bound("...", out, count, max); return; }
    switch (n->type) {
        case JN_NULL: fputs_bound("null", out, count, max); break;
        case JN_BOOL: fputs_bound(n->u.boolean ? "true" : "false", out, count, max); break;
        case JN_NUMBER: {
            char buf[64];
            int k = snprintf(buf, sizeof(buf), "%.17g", n->u.number);
            if (k < 0) k = 0;
            buf[sizeof(buf)-1] = '\0';
            fputs_bound(buf, out, count, max);
        } break;
        case JN_STRING:
            print_escaped_str_bound(n->u.string ? n->u.string : "", out, count, max);
            break;
        case JN_ARRAY: {
            fputc_bound('[', out, count, max);
            for (size_t i = 0; i < n->u.array.count; i++) {
                if (i) fputs_bound(", ", out, count, max);
                json_print_bound(n->u.array.items[i], indent + 2, depth_limit - 1, out, count, max);
            }
            fputc_bound(']', out, count, max);
        } break;
        case JN_OBJECT: {
            fputc_bound('{', out, count, max);
            for (size_t i = 0; i < n->u.object.count; i++) {
                if (i) fputs_bound(", ", out, count, max);
                print_escaped_str_bound(n->u.object.pairs[i].key ? n->u.object.pairs[i].key : "", out, count, max);
                fputs_bound(": ", out, count, max);
                json_print_bound(n->u.object.pairs[i].value, indent + 2, depth_limit - 1, out, count, max);
            }
            fputc_bound('}', out, count, max);
        } break;
    }
}

/* Networking using libcurl */

typedef struct {
    char *data;
    size_t len;
    size_t cap;
    size_t max;
} MemBuf;

static void membuf_init(MemBuf *b, size_t max) {
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
    b->max = max;
}

static void membuf_free(MemBuf *b) {
    if (b->data) {
        /* not sensitive; regular free */
        free(b->data);
    }
    b->data = NULL;
    b->len = 0;
    b->cap = 0;
}

static int membuf_grow(MemBuf *b, size_t add) {
    if (!b) return 0;
    if (add == 0) return 1;
    if (b->len > b->max || add > b->max - b->len) return 0; /* would exceed max */
    size_t need = b->len + add;
    if (need <= b->cap) return 1;
    size_t newcap = b->cap ? b->cap : 4096;
    while (newcap < need) {
        if (newcap > b->max / 2) { newcap = b->max; break; }
        newcap *= 2;
    }
    if (newcap > b->max) newcap = b->max;
    char *p = (char *)realloc(b->data, newcap + 1); /* +1 for NUL */
    if (!p) return 0;
    b->data = p;
    b->cap = newcap;
    return 1;
}

static size_t curl_write_cb(void *ptr, size_t size, size_t nmemb, void *userdata) {
    MemBuf *b = (MemBuf *)userdata;
    if (!ptr || !b) return 0;
    if (size == 0 || nmemb == 0) return 0;
    if (nmemb > SIZE_MAX / size) return 0; /* overflow */
    size_t n = size * nmemb;
    if (!membuf_grow(b, n)) return 0;
    memcpy(b->data + b->len, ptr, n);
    b->len += n;
    b->data[b->len] = '\0';
    return n;
}

static int validate_https_url(const char *url) {
    if (!url) return 0;
    size_t L = strnlen(url, 4097);
    if (L == 0 || L > 4096) return 0;
    const char *scheme = "https://";
    size_t sl = strlen(scheme);
    if (L < sl) return 0;
    if (strncmp(url, scheme, sl) != 0) return 0;
    /* basic additional checks: no spaces, no control chars */
    for (size_t i = 0; i < L; i++) {
        unsigned char c = (unsigned char)url[i];
        if (c <= 0x1F || c == 0x7F || c == ' ') return 0;
    }
    return 1;
}

static int curl_global_inited = 0;

static int init_curl_once(void) {
    if (!curl_global_inited) {
        if (curl_global_init(CURL_GLOBAL_DEFAULT) != 0) {
            return 0;
        }
        curl_global_inited = 1;
    }
    return 1;
}

/*
Fetch a URL over HTTPS with strong defaults.
Parameters:
- url: must be https:// URL
- max_bytes: upper bound on bytes to download
- out_buf: output buffer pointer (malloc'd, NUL-terminated). Caller must free.
- out_len: length of valid bytes in out_buf (excluding NUL)
- errbuf: to receive a short generic error message
Returns 1 on success, 0 on failure.
*/
static int https_fetch_to_mem(const char *url, size_t max_bytes, char **out_buf, size_t *out_len, char *errbuf, size_t errlen) {
    if (out_buf) *out_buf = NULL;
    if (out_len) *out_len = 0;
    if (!validate_https_url(url)) {
        if (errbuf && errlen) snprintf(errbuf, errlen, "invalid input");
        return 0;
    }
    if (max_bytes == 0 || max_bytes > (50U * 1024U * 1024U)) { /* hard cap 50MB */
        if (errbuf && errlen) snprintf(errbuf, errlen, "invalid input");
        return 0;
    }
    if (!init_curl_once()) {
        if (errbuf && errlen) snprintf(errbuf, errlen, "network error");
        return 0;
    }
    CURL *curl = curl_easy_init();
    if (!curl) {
        if (errbuf && errlen) snprintf(errbuf, errlen, "network error");
        return 0;
    }
    MemBuf mb;
    membuf_init(&mb, max_bytes);

    curl_easy_setopt(curl, CURLOPT_URL, url);
#if LIBCURL_VERSION_NUM >= 0x073D00 /* 7.61.0 */
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS_STR, "https");
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS_STR, "https");
#else
    curl_easy_setopt(curl, CURLOPT_PROTOCOLS, (long)CURLPROTO_HTTPS);
    curl_easy_setopt(curl, CURLOPT_REDIR_PROTOCOLS, (long)CURLPROTO_HTTPS);
#endif
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
#ifdef CURL_SSLVERSION_TLSv1_2
    curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
#endif
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "SecureJSONFetcher/1.0 (+https)");
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&mb);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
#if LIBCURL_VERSION_NUM >= 0x072C00 /* 7.44.0 */
    curl_easy_setopt(curl, CURLOPT_UPKEEP_INTERVAL_MS, 10000L);
#endif

    CURLcode rc = curl_easy_perform(curl);
    long http_code = 0;
    if (rc == CURLE_OK) {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    }

    curl_easy_cleanup(curl);

    if (rc != CURLE_OK || http_code < 200 || http_code >= 300) {
        membuf_free(&mb);
        if (errbuf && errlen) snprintf(errbuf, errlen, "network error");
        return 0;
    }

    if (mb.data == NULL) {
        membuf_free(&mb);
        if (errbuf && errlen) snprintf(errbuf, errlen, "network error");
        return 0;
    }

    /* Ensure NUL termination already set in write callback. */
    if (out_buf) *out_buf = mb.data; else membuf_free(&mb);
    if (out_len) *out_len = mb.len;
    return 1;
}

/*
Parse JSON text into JsonNode tree, with limits.
Returns root or NULL on failure.
*/
static JsonNode *parse_json_text(const char *text, size_t len, size_t tokens_max, int depth_limit, char *errbuf, size_t errlen) {
    if (!text || len == 0 || len > (50U * 1024U * 1024U)) {
        if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
        return NULL;
    }
    if (tokens_max == 0 || tokens_max > 500000U) tokens_max = 500000U;
    jsmn_parser p;
    jsmntok_t *toks = NULL;
    size_t tokcap = 256;
    if (tokcap > tokens_max) tokcap = tokens_max;
    int r = JSMN_ERROR_NOMEM;
    for (;;) {
        toks = (jsmntok_t *)malloc(tokcap * sizeof(jsmntok_t));
        if (!toks) {
            if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
            return NULL;
        }
        jsmn_init(&p);
        r = jsmn_parse(&p, text, len, toks, (unsigned int)tokcap);
        if (r == JSMN_ERROR_NOMEM) {
            free(toks);
            if (tokcap >= tokens_max) {
                if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
                return NULL;
            }
            size_t nt = tokcap * 2;
            if (nt > tokens_max) nt = tokens_max;
            tokcap = nt;
            continue;
        }
        break;
    }
    if (r < 0) {
        free(toks);
        if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
        return NULL;
    }
    size_t tokc = (size_t)r;
    if (tokc == 0) {
        free(toks);
        if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
        return NULL;
    }

    TokCursor cur;
    cur.json = text;
    cur.toks = toks;
    cur.tokc = tokc;
    cur.idx = 0;
    cur.depth = 0;
    cur.depth_limit = (depth_limit <= 0 || depth_limit > 10000) ? 1024 : depth_limit;
    cur.node_cap_limit = tokens_max;

    JsonNode *root = build_node(&cur, errbuf, errlen);
    free(toks);
    if (!root) {
        if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
        return NULL;
    }
    /* Ensure we've consumed exactly one root element */
    if (cur.idx != tokc) {
        /* Some extra tokens not consumed - treat as error */
        free_json(root);
        if (errbuf && errlen) snprintf(errbuf, errlen, "parse error");
        return NULL;
    }
    return root;
}

/*
High-level API: fetch and parse JSON from HTTPS URL.
Returns a JsonNode* on success, or NULL on failure. errbuf gets a short generic message.
max_bytes caps the download size to prevent memory abuse.
*/
static JsonNode *fetch_json_from_url(const char *url, size_t max_bytes, char *errbuf, size_t errlen) {
    if (errbuf && errlen) errbuf[0] = '\0';
    char *buf = NULL;
    size_t blen = 0;
    if (!https_fetch_to_mem(url, max_bytes, &buf, &blen, errbuf, errlen)) {
        return NULL;
    }
    /* Basic content sanity: ensure buffer is UTF-8-ish ASCII or binary safe; JSON is text, but we'll just parse. */
    JsonNode *root = parse_json_text(buf, blen, 200000U, 2048, errbuf, errlen);
    free(buf);
    return root;
}

/* Optional: helper to print the root type */
static const char *json_type_name(const JsonNode *n) {
    if (!n) return "null";
    switch (n->type) {
        case JN_OBJECT: return "object";
        case JN_ARRAY: return "array";
        case JN_STRING: return "string";
        case JN_NUMBER: return "number";
        case JN_BOOL: return "bool";
        case JN_NULL: return "null";
        default: return "unknown";
    }
}

/* Main with 5 test cases */
int main(int argc, char **argv) {
    const size_t MAX_BYTES = 1024U * 1024U; /* 1 MiB limit */
    char err[64];

    if (argc >= 2) {
        const char *url = argv[1];
        JsonNode *root = fetch_json_from_url(url, MAX_BYTES, err, sizeof(err));
        if (!root) {
            fprintf(stderr, "Error: %s\n", err[0] ? err : "failed\n");
            return 1;
        }
        printf("Fetched and parsed: %s\n", url);
        printf("Root type: %s\n", json_type_name(root));
        size_t printed = 0;
        json_print_bound(root, 0, 64, stdout, &printed, 2000);
        if (printed >= 2000) printf(" ...");
        printf("\n");
        free_json(root);
        return 0;
    }

    const char *tests[5] = {
        "https://jsonplaceholder.typicode.com/todos/1",
        "https://httpbin.org/json",
        "https://api.github.com/",
        "https://raw.githubusercontent.com/typicode/demo/master/db.json",
        "https://api.github.com/repos/curl/curl"
    };

    for (int i = 0; i < 5; i++) {
        printf("Test #%d: %s\n", i + 1, tests[i]);
        JsonNode *root = fetch_json_from_url(tests[i], MAX_BYTES, err, sizeof(err));
        if (!root) {
            fprintf(stderr, "  Error: %s\n", err[0] ? err : "failed\n");
            continue;
        }
        printf("  Root type: %s\n  Snippet: ", json_type_name(root));
        size_t printed = 0;
        json_print_bound(root, 0, 64, stdout, &printed, 400);
        if (printed >= 400) printf(" ...");
        printf("\n");
        free_json(root);
    }
    return 0;
}