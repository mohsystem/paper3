#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define MAX_STOCK_NAME_LEN 64
#define MAX_QUANTITY 1000000

typedef struct {
    char stockName[MAX_STOCK_NAME_LEN + 1];
    int quantity;
    time_t createdAt;
} Order;

typedef struct {
    Order* orders;
    size_t size;
    size_t capacity;
} InMemoryDatabase;

static void db_init(InMemoryDatabase* db) {
    db->orders = NULL;
    db->size = 0;
    db->capacity = 0;
}

static void db_free(InMemoryDatabase* db) {
    free(db->orders);
    db->orders = NULL;
    db->size = 0;
    db->capacity = 0;
}

static int db_grow(InMemoryDatabase* db) {
    size_t newcap = db->capacity == 0 ? 8 : db->capacity * 2;
    if (newcap < db->capacity) return 0; // overflow guard
    Order* p = (Order*)realloc(db->orders, newcap * sizeof(Order));
    if (!p) return 0;
    db->orders = p;
    db->capacity = newcap;
    return 1;
}

static void trim(const char* in, char* out, size_t outsz) {
    // Trim leading/trailing whitespace and copy into out with bounds
    const char* start = in;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = in + strlen(in);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    if (len >= outsz) len = outsz - 1;
    memcpy(out, start, len);
    out[len] = '\0';
}

static int is_valid_stock_name(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len < 1 || len > MAX_STOCK_NAME_LEN) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char c = (unsigned char)name[i];
        if (!(isalnum(c) || c == ' ' || c == '_' || c == '-' || c == '.')) {
            return 0;
        }
    }
    return 1;
}

static int is_valid_quantity(int q) {
    return q >= 1 && q <= MAX_QUANTITY;
}

static int insert_order(InMemoryDatabase* db, const char* stockName, int quantity) {
    if (db->size == db->capacity) {
        if (!db_grow(db)) return 0;
    }
    Order* o = &db->orders[db->size];
    // Safe copy into fixed buffer
    size_t len = strlen(stockName);
    if (len > MAX_STOCK_NAME_LEN) len = MAX_STOCK_NAME_LEN;
    memcpy(o->stockName, stockName, len);
    o->stockName[len] = '\0';
    o->quantity = quantity;
    o->createdAt = time(NULL);
    db->size++;
    return 1;
}

static void format_time(time_t t, char* buf, size_t bufsz) {
    struct tm tmv;
#if defined(_WIN32)
    localtime_s(&tmv, &t);
#else
    localtime_r(&t, &tmv);
#endif
    strftime(buf, bufsz, "%Y-%m-%d %H:%M:%S", &tmv);
}

static char* stock_view(InMemoryDatabase* db) {
    // Compute required size
    size_t total = 0;
    total += strlen("Stock Orders:\n");
    for (size_t i = 0; i < db->size; i++) {
        // index + ". " + name + " - " + qty + " @ " + time + "\n"
        total += 10 + strlen(db->orders[i].stockName) + 3 + 12 + 3 + 19 + 1;
    }
    if (total < 1) total = 1;
    char* out = (char*)malloc(total + 1);
    if (!out) return NULL;
    size_t off = 0;
    int n = snprintf(out + off, total + 1 - off, "Stock Orders:\n");
    if (n < 0) { free(out); return NULL; }
    off += (size_t)n;
    for (size_t i = 0; i < db->size; i++) {
        char ts[32];
        format_time(db->orders[i].createdAt, ts, sizeof(ts));
        n = snprintf(out + off, total + 1 - off, "%zu. %s - %d @ %s\n",
                     i + 1, db->orders[i].stockName, db->orders[i].quantity, ts);
        if (n < 0) { free(out); return NULL; }
        off += (size_t)n;
        if (off > total) break;
    }
    out[off] = '\0';
    return out;
}

static char* render_form(void) {
    const char* tpl =
        "<!doctype html>\n"
        "<html><head><meta charset=\"utf-8\"><title>Buy Order</title></head>\n"
        "<body>\n"
        "<h1>Buy Order</h1>\n"
        "<form method=\"POST\" action=\"/buy_order\">\n"
        "  <label>Stock Name: <input type=\"text\" name=\"stock\" maxlength=\"64\" required></label><br>\n"
        "  <label>Quantity: <input type=\"number\" name=\"qty\" min=\"1\" max=\"1000000\" required></label><br>\n"
        "  <button type=\"submit\">Submit</button>\n"
        "</form>\n"
        "</body></html>\n";
    size_t len = strlen(tpl);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, tpl, len + 1);
    return out;
}

static char* buy_order(InMemoryDatabase* db, const char* stockNameInput, int quantityInput) {
    char sanitized[MAX_STOCK_NAME_LEN + 1];
    trim(stockNameInput ? stockNameInput : "", sanitized, sizeof(sanitized));
    // Ensure max length
    sanitized[MAX_STOCK_NAME_LEN] = '\0';

    if (!is_valid_stock_name(sanitized)) {
        char* msg = (char*)malloc(32);
        if (!msg) return NULL;
        strcpy(msg, "Error: invalid stock name.");
        return msg;
    }
    if (!is_valid_quantity(quantityInput)) {
        char* msg = (char*)malloc(30);
        if (!msg) return NULL;
        strcpy(msg, "Error: invalid quantity.");
        return msg;
    }
    if (!insert_order(db, sanitized, quantityInput)) {
        char* msg = (char*)malloc(33);
        if (!msg) return NULL;
        strcpy(msg, "Error: failed to insert order.");
        return msg;
    }
    // Build response with redirect simulation and stock_view
    char* sv = stock_view(db);
    if (!sv) {
        char* msg = (char*)malloc(33);
        if (!msg) return NULL;
        strcpy(msg, "Error: failed to render view.");
        return msg;
    }
    const char* head = "OK: Order inserted. Redirecting to /stock_view ...\n";
    size_t total = strlen(head) + strlen(sv) + 1;
    char* out = (char*)malloc(total);
    if (!out) { free(sv); return NULL; }
    snprintf(out, total, "%s%s", head, sv);
    free(sv);
    return out;
}

int main(void) {
    InMemoryDatabase db;
    db_init(&db);

    char* form = render_form();
    if (form) {
        printf("== Rendered Form (Demo) ==\n%s\n", form);
        free(form);
    } else {
        printf("Failed to render form.\n");
    }

    // Test 1: valid
    char* r1 = buy_order(&db, "AAPL", 10);
    if (r1) { printf("Test 1: %s\n", r1); free(r1); }

    // Test 2: valid
    char* r2 = buy_order(&db, "GOOG", 25);
    if (r2) { printf("Test 2: %s\n", r2); free(r2); }

    // Test 3: invalid quantity
    char* r3 = buy_order(&db, "MSFT", 0);
    if (r3) { printf("Test 3: %s\n", r3); free(r3); }

    // Test 4: invalid stock name
    char* r4 = buy_order(&db, "DROP TABLE;--", 5);
    if (r4) { printf("Test 4: %s\n", r4); free(r4); }

    // Test 5: valid with dot and space
    char* r5 = buy_order(&db, "BRK.B Class A", 100);
    if (r5) { printf("Test 5: %s\n", r5); free(r5); }

    db_free(&db);
    return 0;
}