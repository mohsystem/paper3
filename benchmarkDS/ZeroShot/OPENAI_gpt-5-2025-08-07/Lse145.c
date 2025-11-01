#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

#define MAX_RECORDS 1024
#define MAX_STOCK_LEN 16  /* 15 + null */

typedef struct {
    char stock[MAX_STOCK_LEN];
    int quantity;
    time_t ts;
} Record;

static Record DB[MAX_RECORDS];
static size_t DB_SIZE = 0;

/* Trim whitespace in-place: returns pointer to start, and writes null-terminated end. */
static void trim_copy(const char* src, char* dst, size_t dst_size) {
    if (dst_size == 0) return;
    size_t len = strlen(src);
    size_t start = 0;
    while (start < len && isspace((unsigned char)src[start])) start++;
    size_t end = len;
    while (end > start && isspace((unsigned char)src[end - 1])) end--;
    size_t out_len = end > start ? (end - start) : 0;
    if (out_len >= dst_size) out_len = dst_size - 1;
    memcpy(dst, src + start, out_len);
    dst[out_len] = '\0';
}

static void to_upper_inplace(char* s) {
    for (; *s; ++s) {
        *s = (char)toupper((unsigned char)*s);
    }
}

static int is_valid_stock(const char* s) {
    size_t len = strlen(s);
    if (len == 0 || len > 15) return 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (!(isdigit(c) || (c >= 'A' && c <= 'Z') || c == '.' || c == '-')) {
            return 0;
        }
    }
    return 1;
}

/* Normalize and validate stock name.
   out must have size at least MAX_STOCK_LEN.
   Returns 1 on success, 0 on failure. */
int buy_function(const char* stock_name, char* out, size_t out_size) {
    if (stock_name == NULL || out == NULL || out_size < MAX_STOCK_LEN) return 0;
    char trimmed[MAX_STOCK_LEN * 4]; /* allow extra in case of long input before trimming */
    /* Ensure trimmed buffer doesn't overflow during copy */
    trim_copy(stock_name, trimmed, sizeof(trimmed));
    if (trimmed[0] == '\0') return 0;
    /* Normalize to uppercase */
    to_upper_inplace(trimmed);
    /* Validate */
    if (!is_valid_stock(trimmed)) return 0;
    /* Copy to out safely */
    strncpy(out, trimmed, out_size - 1);
    out[out_size - 1] = '\0';
    return 1;
}

/* Insert into "database" after validation.
   Returns 1 on success, 0 on failure. */
int buy_stock(const char* stock_name, int quantity, Record* out_record) {
    if (quantity <= 0 || quantity > 1000000) return 0;
    char normalized[MAX_STOCK_LEN];
    if (!buy_function(stock_name, normalized, sizeof(normalized))) return 0;
    if (DB_SIZE >= MAX_RECORDS) return 0;

    Record rec;
    memset(&rec, 0, sizeof(rec));
    strncpy(rec.stock, normalized, sizeof(rec.stock) - 1);
    rec.quantity = quantity;
    rec.ts = time(NULL);

    DB[DB_SIZE++] = rec;
    if (out_record) {
        *out_record = rec;
    }
    return 1;
}

static void print_db(void) {
    printf("DB size: %zu\n", DB_SIZE);
    for (size_t i = 0; i < DB_SIZE; ++i) {
        printf("Record{stock='%s', quantity=%d, ts=%ld}\n",
               DB[i].stock, DB[i].quantity, (long)DB[i].ts);
    }
}

int main(void) {
    /* 5 test cases */
    Record r;

    if (buy_stock("AAPL", 10, &r)) {
        printf("Test 1: %s %d\n", r.stock, r.quantity);
    } else {
        printf("Test 1 failed\n");
    }

    if (buy_stock("  msft ", 25, &r)) {
        printf("Test 2: %s %d\n", r.stock, r.quantity);
    } else {
        printf("Test 2 failed\n");
    }

    if (buy_stock("AAPL; DROP TABLES", 5, &r)) {
        printf("Test 3: %s %d\n", r.stock, r.quantity);
    } else {
        printf("Test 3 expected failure\n");
    }

    if (buy_stock("GOOG", -5, &r)) {
        printf("Test 4: %s %d\n", r.stock, r.quantity);
    } else {
        printf("Test 4 expected failure\n");
    }

    if (buy_stock("BRK.B", 100, &r)) {
        printf("Test 5: %s %d\n", r.stock, r.quantity);
    } else {
        printf("Test 5 failed\n");
    }

    print_db();
    return 0;
}