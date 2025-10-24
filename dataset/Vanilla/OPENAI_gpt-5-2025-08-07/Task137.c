#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int id;
    const char* name;
    const char* category;
    double price;
    int stock;
} Product;

static const Product PRODUCT_TABLE[] = {
    {1001, "Widget", "Tools", 19.99, 120},
    {1002, "Gadget", "Electronics", 99.50, 55},
    {1003, "Gizmo", "Electronics", 49.00, 200},
    {1004, "Pro Headphones", "Audio", 129.99, 35},
    {1005, "Office Chair", "Furniture", 199.99, 15},
    {1006, "Pro Keyboard", "Computers", 89.99, 60},
};
static const size_t PRODUCT_COUNT = sizeof(PRODUCT_TABLE) / sizeof(PRODUCT_TABLE[0]);

typedef struct {
    char* data;
    size_t len;
    size_t cap;
} StringBuilder;

static void sb_init(StringBuilder* sb) {
    sb->len = 0;
    sb->cap = 256;
    sb->data = (char*)malloc(sb->cap);
    if (sb->data) sb->data[0] = '\0';
}
static void sb_free(StringBuilder* sb) {
    if (sb->data) free(sb->data);
    sb->data = NULL; sb->len = 0; sb->cap = 0;
}
static int sb_ensure(StringBuilder* sb, size_t add) {
    if (sb->len + add + 1 <= sb->cap) return 1;
    size_t ncap = sb->cap * 2;
    while (ncap < sb->len + add + 1) ncap *= 2;
    char* nd = (char*)realloc(sb->data, ncap);
    if (!nd) return 0;
    sb->data = nd; sb->cap = ncap;
    return 1;
}
static int sb_append_str(StringBuilder* sb, const char* s) {
    size_t sl = strlen(s);
    if (!sb_ensure(sb, sl)) return 0;
    memcpy(sb->data + sb->len, s, sl);
    sb->len += sl;
    sb->data[sb->len] = '\0';
    return 1;
}
static int sb_append_char(StringBuilder* sb, char c) {
    if (!sb_ensure(sb, 1)) return 0;
    sb->data[sb->len++] = c;
    sb->data[sb->len] = '\0';
    return 1;
}
static int sb_append_int(StringBuilder* sb, int v) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%d", v);
    return sb_append_str(sb, buf);
}
static int sb_append_double(StringBuilder* sb, double v) {
    char buf[64];
    snprintf(buf, sizeof(buf), "%.2f", v);
    return sb_append_str(sb, buf);
}

static char* json_escape(const char* s) {
    StringBuilder sb; sb_init(&sb);
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        unsigned char c = *p;
        switch (c) {
            case '\\': sb_append_str(&sb, "\\\\"); break;
            case '\"': sb_append_str(&sb, "\\\""); break;
            case '\b': sb_append_str(&sb, "\\b"); break;
            case '\f': sb_append_str(&sb, "\\f"); break;
            case '\n': sb_append_str(&sb, "\\n"); break;
            case '\r': sb_append_str(&sb, "\\r"); break;
            case '\t': sb_append_str(&sb, "\\t"); break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    sb_append_str(&sb, buf);
                } else {
                    sb_append_char(&sb, (char)c);
                }
        }
    }
    return sb.data; // caller frees
}

static char* strtolower_dup(const char* s) {
    size_t n = strlen(s);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < n; ++i) out[i] = (char)tolower((unsigned char)s[i]);
    out[n] = '\0';
    return out;
}

static char* trim_dup(const char* s) {
    const char* p = s;
    while (*p && isspace((unsigned char)*p)) p++;
    const char* q = s + strlen(s);
    while (q > p && isspace((unsigned char)q[-1])) q--;
    size_t n = (size_t)(q - p);
    char* out = (char*)malloc(n + 1);
    if (!out) return NULL;
    memcpy(out, p, n);
    out[n] = '\0';
    return out;
}

static int ci_equal(const char* a, const char* b) {
    while (*a && *b) {
        unsigned char ca = (unsigned char)tolower((unsigned char)*a++);
        unsigned char cb = (unsigned char)tolower((unsigned char)*b++);
        if (ca != cb) return 0;
    }
    return *a == '\0' && *b == '\0';
}

static int ci_contains(const char* haystack, const char* needle) {
    char* h = strtolower_dup(haystack);
    char* n = strtolower_dup(needle);
    if (!h || !n) { if (h) free(h); if (n) free(n); return 0; }
    int found = strstr(h, n) != NULL;
    free(h); free(n);
    return found;
}

static char* products_to_json(const Product* arr, const size_t* idxs, size_t count) {
    StringBuilder sb; sb_init(&sb);
    sb_append_char(&sb, '[');
    for (size_t i = 0; i < count; ++i) {
        const Product* p = &arr[idxs[i]];
        sb_append_char(&sb, '{');
        sb_append_str(&sb, "\"id\":"); sb_append_int(&sb, p->id); sb_append_char(&sb, ',');
        sb_append_str(&sb, "\"name\":\""); char* n = json_escape(p->name); sb_append_str(&sb, n); free(n); sb_append_str(&sb, "\",");
        sb_append_str(&sb, "\"category\":\""); char* c = json_escape(p->category); sb_append_str(&sb, c); free(c); sb_append_str(&sb, "\",");
        sb_append_str(&sb, "\"price\":"); sb_append_double(&sb, p->price); sb_append_char(&sb, ',');
        sb_append_str(&sb, "\"stock\":"); sb_append_int(&sb, p->stock);
        sb_append_char(&sb, '}');
        if (i + 1 < count) sb_append_char(&sb, ',');
    }
    sb_append_char(&sb, ']');
    return sb.data; // caller frees
}

char* process_input(const char* input) {
    if (!input) {
        char* out = (char*)malloc(3);
        strcpy(out, "[]");
        return out;
    }
    char* t = trim_dup(input);
    if (!t || t[0] == '\0') {
        if (t) free(t);
        char* out = (char*)malloc(3);
        strcpy(out, "[]");
        return out;
    }

    char* tl = strtolower_dup(t);
    if (tl && strcmp(tl, "all") == 0) {
        free(tl);
        free(t);
        size_t* idxs = (size_t*)malloc(PRODUCT_COUNT * sizeof(size_t));
        for (size_t i = 0; i < PRODUCT_COUNT; ++i) idxs[i] = i;
        char* json = products_to_json(PRODUCT_TABLE, idxs, PRODUCT_COUNT);
        free(idxs);
        return json;
    }
    if (tl) free(tl);

    char* eq = strchr(t, '=');
    if (!eq) {
        free(t);
        char* out = (char*)malloc(3);
        strcpy(out, "[]");
        return out;
    }
    *eq = '\0';
    char* key_raw = trim_dup(t);
    char* val_raw = trim_dup(eq + 1);
    free(t);

    char* key = strtolower_dup(key_raw ? key_raw : "");
    if (key_raw) free(key_raw);

    size_t* idxs = (size_t*)malloc(PRODUCT_COUNT * sizeof(size_t));
    size_t count = 0;

    if (strcmp(key, "id") == 0) {
        char* endp = NULL;
        long id = strtol(val_raw ? val_raw : "", &endp, 10);
        if (endp && *endp == '\0') {
            for (size_t i = 0; i < PRODUCT_COUNT; ++i) {
                if (PRODUCT_TABLE[i].id == (int)id) { idxs[count++] = i; break; }
            }
        }
    } else if (strcmp(key, "name") == 0) {
        for (size_t i = 0; i < PRODUCT_COUNT; ++i) {
            if (ci_equal(PRODUCT_TABLE[i].name, val_raw ? val_raw : "")) idxs[count++] = i;
        }
    } else if (strcmp(key, "category") == 0) {
        for (size_t i = 0; i < PRODUCT_COUNT; ++i) {
            if (ci_equal(PRODUCT_TABLE[i].category, val_raw ? val_raw : "")) idxs[count++] = i;
        }
    } else if (strcmp(key, "search") == 0) {
        for (size_t i = 0; i < PRODUCT_COUNT; ++i) {
            if (ci_contains(PRODUCT_TABLE[i].name, val_raw ? val_raw : "")) idxs[count++] = i;
        }
    } else {
        // invalid key -> empty
    }

    if (val_raw) free(val_raw);
    free(key);

    char* json = products_to_json(PRODUCT_TABLE, idxs, count);
    free(idxs);
    return json;
}

int main(void) {
    const char* tests[5] = {
        "id=1002",
        "name=Gizmo",
        "category=Audio",
        "search=Pro",
        "all"
    };
    for (int i = 0; i < 5; ++i) {
        printf("Query: %s\n", tests[i]);
        char* out = process_input(tests[i]);
        printf("%s\n", out);
        free(out);
    }
    return 0;
}