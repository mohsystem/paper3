#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

typedef struct {
    int id;
    char* stock;
    int qty;
    long ts;
} Record;

#define DB_CAP 100
static Record db[DB_CAP];
static int db_count = 0;
static int next_id = 1;

static char* str_trim(const char* s) {
    if (!s) return NULL;
    const char* start = s;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = s + strlen(s);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    memcpy(out, start, len);
    out[len] = '\0';
    return out;
}

static char* str_upper_dup(const char* s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[i] = (char)toupper((unsigned char)s[i]);
    }
    out[len] = '\0';
    return out;
}

char* buy_function(const char* stockName) {
    if (!stockName) return NULL;
    char* trimmed = str_trim(stockName);
    if (!trimmed) return NULL;
    if (trimmed[0] == '\0') {
        free(trimmed);
        return NULL;
    }
    char* upper = str_upper_dup(trimmed);
    free(trimmed);
    return upper; // caller must free
}

int buy_stock(const char* stockName, int quantity) {
    char* normalized = buy_function(stockName);
    if (!normalized || quantity <= 0) {
        if (normalized) free(normalized);
        return -1;
    }
    if (db_count >= DB_CAP) {
        free(normalized);
        return -1;
    }
    Record rec;
    rec.id = next_id++;
    rec.stock = normalized; // take ownership
    rec.qty = quantity;
    rec.ts = time(NULL);
    db[db_count++] = rec;
    return rec.id;
}

void print_db() {
    for (int i = 0; i < db_count; ++i) {
        printf("ID=%d Stock=%s Qty=%d Ts=%ld\n", db[i].id, db[i].stock, db[i].qty, db[i].ts);
    }
}

int main() {
    int id1 = buy_stock("AAPL", 10);
    int id2 = buy_stock("goog", 5);
    int id3 = buy_stock("  msft  ", 20);
    int id4 = buy_stock("", 7);      // invalid
    int id5 = buy_stock("TSLA", -3); // invalid

    printf("Inserted IDs: %d %d %d %d %d\n", id1, id2, id3, id4, id5);
    printf("Database contents:\n");
    print_db();

    // free allocated strings
    for (int i = 0; i < db_count; ++i) {
        free(db[i].stock);
    }
    return 0;
}