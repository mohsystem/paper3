#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/*
 Steps applied:
 1) Problem understanding
 2) Security requirements
 3) Secure coding generation
 4) Code review
 5) Secure code output
*/

typedef struct {
    int id;
    const char* name;
    double price;
    int stock;
} Product;

typedef struct {
    const Product* items;
    size_t count;
} ProductRepository;

/* Repository data */
static const Product PRODUCTS[] = {
    {1001, "Wireless Mouse", 25.99, 42},
    {1002, "Mechanical Keyboard", 79.49, 12},
    {1003, "USB-C Cable", 9.99, 150},
    {1004, "27-inch Monitor", 199.99, 8},
    {1005, "Webcam Pro", 49.95, 5}
};

static ProductRepository build_repository(void) {
    ProductRepository repo;
    repo.items = PRODUCTS;
    repo.count = sizeof(PRODUCTS)/sizeof(PRODUCTS[0]);
    return repo;
}

/* Helpers */
static int is_allowed_char(int c) {
    return isalnum(c) || c==' ' || c=='_' || c=='-';
}

static char* normalize_name(const char* input) {
    if (!input) return NULL;

    /* Trim */
    const char* start = input;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = input + strlen(input);
    while (end > start && isspace((unsigned char)*(end-1))) end--;

    size_t len = (size_t)(end - start);
    if (len == 0) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }

    /* Validate allowed chars and collapse spaces */
    char* tmp = (char*)malloc(len + 1);
    if (!tmp) return NULL;
    size_t j = 0;
    int prev_space = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)start[i];
        if (!is_allowed_char(c) && !isspace(c)) {
            free(tmp);
            return NULL;
        }
        if (isspace(c)) {
            if (!prev_space) {
                tmp[j++] = ' ';
                prev_space = 1;
            }
        } else {
            tmp[j++] = (char)tolower(c);
            prev_space = 0;
        }
    }
    tmp[j] = '\0';

    /* Trim potential trailing space after collapse */
    if (j > 0 && tmp[j-1] == ' ') {
        tmp[j-1] = '\0';
    }
    return tmp;
}

static const Product* find_by_id_secure(const ProductRepository* repo, int id) {
    if (!repo) return NULL;
    if (id < 0 || id > 1000000000) return NULL;
    for (size_t i = 0; i < repo->count; i++) {
        if (repo->items[i].id == id) return &repo->items[i];
    }
    return NULL;
}

static const Product* find_by_name_secure(const ProductRepository* repo, const char* name) {
    if (!repo || !name) return NULL;
    char* norm = normalize_name(name);
    if (!norm) return NULL;
    const Product* found = NULL;
    for (size_t i = 0; i < repo->count; i++) {
        char* repo_norm = normalize_name(repo->items[i].name);
        if (!repo_norm) continue;
        int match = (strcmp(repo_norm, norm) == 0);
        free(repo_norm);
        if (match) {
            found = &repo->items[i];
            break;
        }
    }
    free(norm);
    return found;
}

static char* escape_json(const char* s) {
    if (!s) {
        char* r = (char*)malloc(3);
        if (r) strcpy(r, "");
        return r;
    }
    size_t len = strlen(s);
    /* Worst case every char escapes to 6 chars; allocate sufficient buffer */
    size_t cap = len * 6 + 1;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        switch (c) {
            case '\"': out[j++]='\\'; out[j++]='\"'; break;
            case '\\': out[j++]='\\'; out[j++]='\\'; break;
            case '\b': out[j++]='\\'; out[j++]='b'; break;
            case '\f': out[j++]='\\'; out[j++]='f'; break;
            case '\n': out[j++]='\\'; out[j++]='n'; break;
            case '\r': out[j++]='\\'; out[j++]='r'; break;
            case '\t': out[j++]='\\'; out[j++]='t'; break;
            default:
                if (c < 0x20) {
                    /* \u00XX */
                    int n = snprintf(out + j, cap - j, "\\u%04x", c);
                    if (n < 0) { free(out); return NULL; }
                    j += (size_t)n;
                } else {
                    out[j++] = (char)c;
                }
        }
    }
    out[j] = '\0';
    return out;
}

static char* to_json(const Product* p) {
    if (!p) return NULL;
    char priceBuf[64];
    snprintf(priceBuf, sizeof(priceBuf), "%.2f", p->price);

    char* nameEsc = escape_json(p->name);
    if (!nameEsc) return NULL;

    char buf[256];
    int n = snprintf(buf, sizeof(buf), "{\"id\":%d,\"name\":\"%s\",\"price\":%s,\"stock\":%d}",
                     p->id, nameEsc, priceBuf, p->stock);
    free(nameEsc);
    if (n < 0) return NULL;

    size_t need = (size_t)n + 1;
    char* out = (char*)malloc(need);
    if (!out) return NULL;
    memcpy(out, buf, need);
    return out;
}

static char* json_error(const char* msg) {
    char* esc = escape_json(msg ? msg : "Error");
    if (!esc) return NULL;
    size_t need = strlen(esc) + 12;
    char* out = (char*)malloc(need);
    if (!out) { free(esc); return NULL; }
    snprintf(out, need, "{\"error\":\"%s\"}", esc);
    free(esc);
    return out;
}

static char* json_not_found(void) {
    const char* msg = "{\"message\":\"Product not found\"}";
    size_t n = strlen(msg) + 1;
    char* out = (char*)malloc(n);
    if (!out) return NULL;
    memcpy(out, msg, n);
    return out;
}

/* Public API: process user input and return JSON result (heap-allocated) */
char* process_user_input(const ProductRepository* repo, const char* input) {
    if (!input) return json_error("Invalid input");
    if (strlen(input) > 1024) return json_error("Input too long");

    /* Copy input to a buffer for safe manipulation */
    char buf[1100];
    size_t inlen = strlen(input);
    if (inlen >= sizeof(buf)) return json_error("Input too long");
    memcpy(buf, input, inlen + 1);

    /* Trim buffer */
    size_t start = 0, end = inlen;
    while (start < end && isspace((unsigned char)buf[start])) start++;
    while (end > start && isspace((unsigned char)buf[end-1])) end--;
    buf[end] = '\0';

    char* trimmed = buf + start;

    /* Find colon */
    char* colon = strchr(trimmed, ':');
    if (!colon) return json_error("Unsupported query. Use 'id:<digits>' or 'name:<text>'");

    /* Split key and value */
    *colon = '\0';
    char* key = trimmed;
    char* value = colon + 1;

    /* Trim key and value */
    while (*key && isspace((unsigned char)*key)) key++;
    char* keyEnd = key + strlen(key);
    while (keyEnd > key && isspace((unsigned char)*(keyEnd-1))) keyEnd--;
    *keyEnd = '\0';

    while (*value && isspace((unsigned char)*value)) value++;
    char* valEnd = value + strlen(value);
    while (valEnd > value && isspace((unsigned char)*(valEnd-1))) valEnd--;
    *valEnd = '\0';

    /* Lowercase key */
    for (char* p = key; *p; ++p) *p = (char)tolower((unsigned char)*p);

    if (strcmp(key, "id") == 0) {
        /* Validate digits */
        size_t vlen = strlen(value);
        if (vlen == 0 || vlen > 9) return json_error("Invalid ID");
        for (size_t i = 0; i < vlen; i++) {
            if (!isdigit((unsigned char)value[i])) return json_error("Invalid ID");
        }
        long id = strtol(value, NULL, 10);
        if (id < 0 || id > 1000000000L) return json_error("Invalid ID");
        const Product* p = find_by_id_secure(repo, (int)id);
        if (!p) return json_not_found();
        return to_json(p);
    } else if (strcmp(key, "name") == 0) {
        const Product* p = find_by_name_secure(repo, value);
        if (!p) return json_not_found();
        return to_json(p);
    } else {
        return json_error("Unsupported query. Use 'id:<digits>' or 'name:<text>'");
    }
}

/* 5 test cases in main */
int main(void) {
    ProductRepository repo = build_repository();
    const char* tests[5] = {
        "id:1001",
        "name:Wireless Mouse",
        "name:   usb-c   cable ",
        "id:9999",
        "name:DROP TABLE products;--"
    };

    for (int i = 0; i < 5; i++) {
        char* res = process_user_input(&repo, tests[i]);
        printf("Query: %s\n", tests[i]);
        printf("Result: %s\n", res ? res : "{\"error\":\"Allocation failed\"}");
        free(res);
    }
    return 0;
}