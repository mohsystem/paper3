#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    int id;
    const char* name;
    double price;
    int stock;
} Product;

typedef struct {
    const Product* items;
    size_t count;
} ProductDB;

static Product PRODUCTS[] = {
    {1001, "Widget", 9.99, 100},
    {1002, "Gadget", 12.49, 50},
    {1003, "Thingamajig", 7.95, 0},
    {2001, "Doodad", 5.00, 25},
    {3001, "Sprocket", 15.75, 10}
};

static ProductDB get_db(void) {
    ProductDB db;
    db.items = PRODUCTS;
    db.count = sizeof(PRODUCTS) / sizeof(PRODUCTS[0]);
    return db;
}

static const Product* find_by_id(ProductDB db, int id) {
    for (size_t i = 0; i < db.count; i++) {
        if (db.items[i].id == id) return &db.items[i];
    }
    return NULL;
}

static const Product* find_by_name(ProductDB db, const char* name) {
    for (size_t i = 0; i < db.count; i++) {
        if (strcmp(db.items[i].name, name) == 0) return &db.items[i];
    }
    return NULL;
}

static int is_valid_digits(const char* s) {
    size_t len = strlen(s);
    if (len == 0 || len > 9) return 0;
    for (size_t i = 0; i < len; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

static int is_valid_name(const char* s) {
    size_t len = strlen(s);
    if (len == 0 || len > 50) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)s[i];
        if (!(isalnum(c) || c == ' ' || c == '-' || c == '_')) {
            return 0;
        }
    }
    return 1;
}

static void append_str(char* dst, size_t dst_size, size_t* idx, const char* src) {
    if (*idx >= dst_size) return;
    size_t rem = dst_size - *idx;
    int n = snprintf(dst + *idx, rem, "%s", src);
    if (n < 0) return;
    if ((size_t)n >= rem) {
        *idx = dst_size;
    } else {
        *idx += (size_t)n;
    }
}

static void append_escaped_json(char* dst, size_t dst_size, size_t* idx, const char* src) {
    for (size_t i = 0; src[i] != '\0'; i++) {
        unsigned char c = (unsigned char)src[i];
        char buf[8];
        if (*idx >= dst_size) return;
        switch (c) {
            case '\\': append_str(dst, dst_size, idx, "\\\\"); break;
            case '\"': append_str(dst, dst_size, idx, "\\\""); break;
            case '\b': append_str(dst, dst_size, idx, "\\b"); break;
            case '\f': append_str(dst, dst_size, idx, "\\f"); break;
            case '\n': append_str(dst, dst_size, idx, "\\n"); break;
            case '\r': append_str(dst, dst_size, idx, "\\r"); break;
            case '\t': append_str(dst, dst_size, idx, "\\t"); break;
            default:
                if (c < 0x20) {
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    append_str(dst, dst_size, idx, buf);
                } else {
                    dst[(*idx)++] = (char)c;
                    if (*idx >= dst_size) return;
                    dst[*idx] = '\0';
                }
        }
    }
}

static char* ok_json(const Product* p) {
    // Allocate sufficient buffer for JSON output
    size_t cap = 512;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    out[0] = '\0';
    size_t idx = 0;
    append_str(out, cap, &idx, "{\"ok\":true,\"product\":{\"id\":");
    char numbuf[64];
    snprintf(numbuf, sizeof(numbuf), "%d", p->id);
    append_str(out, cap, &idx, numbuf);
    append_str(out, cap, &idx, ",\"name\":\"");
    append_escaped_json(out, cap, &idx, p->name);
    append_str(out, cap, &idx, "\",\"price\":");
    snprintf(numbuf, sizeof(numbuf), "%.2f", p->price);
    append_str(out, cap, &idx, numbuf);
    append_str(out, cap, &idx, ",\"stock\":");
    snprintf(numbuf, sizeof(numbuf), "%d", p->stock);
    append_str(out, cap, &idx, numbuf);
    append_str(out, cap, &idx, "}}");
    return out;
}

static char* error_json(const char* msg) {
    size_t cap = 256;
    char* out = (char*)malloc(cap);
    if (!out) return NULL;
    size_t idx = 0;
    append_str(out, cap, &idx, "{\"ok\":false,\"error\":\"");
    append_escaped_json(out, cap, &idx, msg);
    append_str(out, cap, &idx, "\"}");
    return out;
}

char* process_query(const char* query) {
    if (query == NULL) {
        return error_json("Query must not be null");
    }
    size_t qlen = strlen(query);
    if (qlen == 0 || qlen > 100) {
        return error_json("Query length must be between 1 and 100");
    }

    ProductDB db = get_db();

    if (strncmp(query, "id=", 3) == 0) {
        const char* idStr = query + 3;
        if (!is_valid_digits(idStr)) {
            return error_json("Invalid id format. Expected 1-9 digits.");
        }
        // Parse id safely
        long long val = 0;
        for (size_t i = 0; idStr[i] != '\0'; i++) {
            val = val * 10 + (idStr[i] - '0');
            if (val > 1000000000LL) break;
        }
        if (val <= 0 || val > 1000000000LL) {
            return error_json("Id out of allowed range");
        }
        int id = (int)val;
        const Product* p = find_by_id(db, id);
        if (!p) return error_json("Product not found");
        return ok_json(p);
    } else if (strncmp(query, "name=", 5) == 0) {
        const char* name = query + 5;
        if (!is_valid_name(name)) {
            return error_json("Invalid name. Allowed: A-Z a-z 0-9 space - _ (1-50 chars)");
        }
        const Product* p = find_by_name(db, name);
        if (!p) return error_json("Product not found");
        return ok_json(p);
    } else {
        return error_json("Invalid query. Use id=<digits> or name=<allowed_name>");
    }
}

int main(void) {
    const char* tests[5] = {
        "id=1002",
        "name=Widget",
        "id=9999",
        "name=Invalid*Name!",
        "foo=bar"
    };

    for (int i = 0; i < 5; i++) {
        char* result = process_query(tests[i]);
        if (result) {
            printf("Query: %s\n", tests[i]);
            printf("Result: %s\n", result);
            printf("---\n");
            free(result);
        } else {
            printf("Allocation error\n");
        }
    }
    return 0;
}