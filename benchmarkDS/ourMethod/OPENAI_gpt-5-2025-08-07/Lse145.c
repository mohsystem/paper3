#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define MAX_NAME_LEN 15
#define MAX_RECORDS 1000000UL

typedef struct {
    unsigned long id;
    char stock[MAX_NAME_LEN + 1];
    int quantity;
} Purchase;

typedef struct {
    Purchase* items;
    size_t size;
    size_t capacity;
    unsigned long next_id;
} Database;

static int db_init(Database* db) {
    if (db == NULL) return 0;
    db->items = (Purchase*)calloc(16, sizeof(Purchase));
    if (db->items == NULL) return 0;
    db->size = 0;
    db->capacity = 16;
    db->next_id = 1UL;
    return 1;
}

static void db_free(Database* db) {
    if (db == NULL) return;
    free(db->items);
    db->items = NULL;
    db->size = 0;
    db->capacity = 0;
    db->next_id = 1UL;
}

static int db_grow(Database* db) {
    if (db->capacity >= MAX_RECORDS) return 0;
    size_t new_cap = db->capacity * 2;
    if (new_cap > MAX_RECORDS) new_cap = (size_t)MAX_RECORDS;
    Purchase* tmp = (Purchase*)realloc(db->items, new_cap * sizeof(Purchase));
    if (tmp == NULL) return 0;
    // zero out new area to keep things clean (not strictly required)
    if (new_cap > db->capacity) {
        size_t diff = new_cap - db->capacity;
        memset(tmp + db->capacity, 0, diff * sizeof(Purchase));
    }
    db->items = tmp;
    db->capacity = new_cap;
    return 1;
}

static int db_insert(Database* db, const char* stock, int quantity, unsigned long* out_id) {
    if (db == NULL || stock == NULL || out_id == NULL) return 0;
    if (db->size >= db->capacity) {
        if (!db_grow(db)) return 0;
    }
    if (db->size >= MAX_RECORDS) return 0;

    Purchase* p = &db->items[db->size];
    p->id = db->next_id++;
    p->quantity = quantity;
    // Safe copy
    size_t n = strnlen(stock, MAX_NAME_LEN);
    memcpy(p->stock, stock, n);
    p->stock[n] = '\0';

    db->size += 1;
    *out_id = p->id;
    return 1;
}

static void rtrim_inplace(char* s) {
    if (s == NULL) return;
    size_t len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        s[len - 1] = '\0';
        len--;
    }
}

static const char* lskip_spaces(const char* s) {
    while (*s && isspace((unsigned char)*s)) s++;
    return s;
}

static int is_valid_ticker_format(const char* s) {
    size_t len = strlen(s);
    if (len == 0 || len > MAX_NAME_LEN) return 0;
    char c0 = s[0];
    if (!(c0 >= 'A' && c0 <= 'Z')) return 0;
    for (size_t i = 1; i < len; i++) {
        char c = s[i];
        int ok = ((c >= 'A' && c <= 'Z') ||
                  (c >= '0' && c <= '9') ||
                  c == '.' || c == '-');
        if (!ok) return 0;
    }
    return 1;
}

static int sanitize_and_validate_stock_name(const char* input, char* out, size_t out_len) {
    if (input == NULL || out == NULL || out_len == 0) return 0;

    // Trim
    while (*input && isspace((unsigned char)*input)) input++;
    size_t len = strnlen(input, 512);
    char buf[512];
    if (len >= sizeof(buf)) return 0;
    memcpy(buf, input, len);
    buf[len] = '\0';
    rtrim_inplace(buf);

    if (buf[0] == '\0') return 0;

    // Uppercase and copy within limit
    size_t out_i = 0;
    for (size_t i = 0; buf[i] != '\0' && out_i + 1 < out_len && out_i < MAX_NAME_LEN; i++) {
        unsigned char c = (unsigned char)buf[i];
        char up = (char)toupper(c);
        out[out_i++] = up;
    }
    out[out_i] = '\0';

    if (!is_valid_ticker_format(out)) return 0;
    return 1;
}

static int is_valid_quantity(int q) {
    return q >= 1 && q <= 1000000;
}

// buy_function: checks if ticker is allowed
static int buy_function(const char* stock_name) {
    static const char* allowed[] = { "AAPL", "GOOG", "MSFT", "TSLA", "AMZN" };
    for (size_t i = 0; i < sizeof(allowed)/sizeof(allowed[0]); i++) {
        if (strcmp(stock_name, allowed[i]) == 0) return 1;
    }
    return 0;
}

// buy_stock: validate input, call buy_function, insert to DB
static int buy_stock(Database* db, const char* stock_input, int quantity, unsigned long* out_id) {
    if (db == NULL || out_id == NULL) return 0;
    if (!is_valid_quantity(quantity)) {
        return 0;
    }
    char sanitized[MAX_NAME_LEN + 1];
    if (!sanitize_and_validate_stock_name(stock_input, sanitized, sizeof(sanitized))) {
        return 0;
    }
    if (!buy_function(sanitized)) {
        return 0;
    }
    if (!db_insert(db, sanitized, quantity, out_id)) {
        return 0;
    }
    return 1;
}

int main(void) {
    Database db;
    if (!db_init(&db)) {
        fprintf(stderr, "Failed to initialize database\n");
        return 1;
    }

    struct { const char* s; int q; } tests[5] = {
        {"AAPL", 10},
        {" msft ", 5},
        {"AAPL;DELETE", 1},
        {"GOOG", 0},
        {"IBM", 1}
    };

    for (int i = 0; i < 5; i++) {
        unsigned long id = 0;
        int ok = buy_stock(&db, tests[i].s, tests[i].q, &id);
        if (ok) {
            printf("Test %d: SUCCESS: id=%lu\n", i+1, id);
        } else {
            printf("Test %d: ERROR\n", i+1);
        }
    }

    for (size_t i = 0; i < db.size; i++) {
        printf("Record: id=%lu stock=%s qty=%d\n",
               db.items[i].id, db.items[i].stock, db.items[i].quantity);
    }

    db_free(&db);
    return 0;
}