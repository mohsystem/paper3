#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define USERNAME_MIN 3
#define USERNAME_MAX 32
#define STOCK_MIN 1
#define STOCK_MAX 16
#define QTY_MIN 1
#define QTY_MAX 1000000

static const char *DB_PATH = "orders_db.csv";
static const char *HEADER = "timestamp,username,stock,quantity";

static int is_valid_username(const char *s) {
    if (s == NULL) return 0;
    size_t len = 0;
    while (s[len] != '\0' && len <= USERNAME_MAX) len++;
    if (len < USERNAME_MIN || len > USERNAME_MAX) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (!(isalnum((unsigned char)c) || c == '_')) return 0;
    }
    return 1;
}

static int is_valid_stock(const char *s) {
    if (s == NULL) return 0;
    size_t len = 0;
    while (s[len] != '\0' && len <= STOCK_MAX) len++;
    if (len < STOCK_MIN || len > STOCK_MAX) return 0;
    for (size_t i = 0; i < len; i++) {
        char c = s[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '-')) return 0;
    }
    return 1;
}

static int is_valid_quantity(int q) {
    return q >= QTY_MIN && q <= QTY_MAX;
}

static int file_exists_nonempty(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return 0;
    int res = 0;
    if (fseek(f, 0, SEEK_END) == 0) {
        long sz = ftell(f);
        if (sz > 0) res = 1;
    }
    fclose(f);
    return res;
}

static int ensure_db_with_header(const char *path) {
    if (!file_exists_nonempty(path)) {
        FILE *f = fopen(path, "ab");
        if (!f) return 0;
        if (fwrite(HEADER, 1, strlen(HEADER), f) != strlen(HEADER)) {
            fclose(f);
            return 0;
        }
        if (fwrite("\n", 1, 1, f) != 1) {
            fclose(f);
            return 0;
        }
        fclose(f);
    }
    return 1;
}

static int append_order(const char *username, const char *stock, int qty) {
    if (!ensure_db_with_header(DB_PATH)) return 0;

    // Build timestamp in UTC: YYYY-MM-DDTHH:MM:SSZ
    time_t now = time(NULL);
    struct tm gmt;
#if defined(_WIN32)
    gmtime_s(&gmt, &now);
#else
    gmtime_r(&now, &gmt);
#endif
    char ts[32];
    if (strftime(ts, sizeof(ts), "%Y-%m-%dT%H:%M:%SZ", &gmt) == 0) {
        return 0;
    }

    // Compose line
    size_t ulen = strlen(username);
    size_t slen = strlen(stock);
    char qtybuf[32];
    snprintf(qtybuf, sizeof(qtybuf), "%d", qty);

    size_t needed = strlen(ts) + 1 + ulen + 1 + slen + 1 + strlen(qtybuf) + 1; // commas + newline
    char *line = (char *)malloc(needed + 1);
    if (!line) return 0;

    int n = snprintf(line, needed + 1, "%s,%s,%s,%s\n", ts, username, stock, qtybuf);
    if (n <= 0 || (size_t)n > needed) {
        free(line);
        return 0;
    }

    FILE *f = fopen(DB_PATH, "ab");
    if (!f) {
        free(line);
        return 0;
    }
    size_t wr = fwrite(line, 1, (size_t)n, f);
    int ok = (wr == (size_t)n);
    fclose(f);
    free(line);
    return ok;
}

// Returns a newly allocated string on success (redirect URL) or NULL on failure.
// Caller must free the returned string if not NULL.
char* process_buy_order(const char* username, const char* stock_name, int stock_quantity) {
    if (!is_valid_username(username)) return NULL;
    if (!is_valid_stock(stock_name)) return NULL;
    if (!is_valid_quantity(stock_quantity)) return NULL;

    // Normalize stock to uppercase
    char stock_norm[STOCK_MAX + 1];
    size_t slen = strlen(stock_name);
    if (slen > STOCK_MAX) return NULL; // double-check
    for (size_t i = 0; i < slen; i++) {
        stock_norm[i] = (char)toupper((unsigned char)stock_name[i]);
    }
    stock_norm[slen] = '\0';

    if (!append_order(username, stock_norm, stock_quantity)) return NULL;

    // Build redirect URL
    const char *prefix = "/users/";
    const char *suffix = "/profile";
    size_t ulen = strlen(username);
    size_t needed = strlen(prefix) + ulen + strlen(suffix) + 1;
    char *result = (char *)malloc(needed);
    if (!result) return NULL;
    snprintf(result, needed, "%s%s%s", prefix, username, suffix);
    return result;
}

static void run_test(const char* user, const char* stock, int qty) {
    char *res = process_buy_order(user, stock, qty);
    printf("Input: user=%s, stock=%s, qty=%d\n", user, stock, qty);
    if (res) {
        printf("Result: %s\n", res);
        free(res);
    } else {
        printf("Result: ERROR\n");
    }
    printf("----\n");
}

int main(void) {
    // 5 test cases
    run_test("alice_01", "AAPL", 10);
    run_test("bob", "GOOG", 100);
    run_test("charlie", "msft", 5000);
    run_test("bad name", "TSLA", 10);  // invalid username
    run_test("dave", "BRK.B", 0);      // invalid quantity
    return 0;
}