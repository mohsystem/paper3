#include <sqlite3.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

static const char* DB_PATH = "lse146_orders_c.db";

static int is_valid_username(const char* s) {
    if (!s) return 0;
    size_t len = strlen(s);
    if (len < 1 || len > 64) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int is_valid_stock_name(const char* s) {
    if (!s) return 0;
    size_t len = strlen(s);
    if (len < 1 || len > 64) return 0;
    for (size_t i = 0; i < len; ++i) {
        char c = s[i];
        if (!(isalnum((unsigned char)c) || c == ' ' || c == '.' || c == '_' || c == '-')) return 0;
    }
    return 1;
}

static int url_encode(const char* src, char* dst, size_t dst_size) {
    // Encode to /profile/<encoded username>
    size_t di = 0;
    for (size_t i = 0; src[i] != '\0'; ++i) {
        unsigned char c = (unsigned char)src[i];
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            if (di + 1 >= dst_size) return 0;
            dst[di++] = (char)c;
        } else {
            if (di + 3 >= dst_size) return 0;
            static const char hex[] = "0123456789ABCDEF";
            dst[di++] = '%';
            dst[di++] = hex[(c >> 4) & 0xF];
            dst[di++] = hex[c & 0xF];
        }
    }
    if (di >= dst_size) return 0;
    dst[di] = '\0';
    return 1;
}

static int ensure_table_exists(sqlite3* db) {
    const char* sql =
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "stock_name TEXT NOT NULL,"
        "quantity INTEGER NOT NULL CHECK(quantity > 0),"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP)";
    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        return 0;
    }
    return 1;
}

// Returns 0 on success and writes redirect URL to out_redirect; non-zero on failure
int place_buy_order(const char* username, const char* stock_name, int stock_quantity, char* out_redirect, size_t out_redirect_size) {
    if (!is_valid_username(username) || !is_valid_stock_name(stock_name)) {
        return -1;
    }
    if (stock_quantity < 1 || stock_quantity > 1000000) {
        return -2;
    }

    sqlite3* db = NULL;
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return -3;
    }

    if (!ensure_table_exists(db)) {
        sqlite3_close(db);
        return -4;
    }

    const char* sql = "INSERT INTO buy_orders(username, stock_name, quantity) VALUES(?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);
        return -5;
    }

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, stock_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, stock_quantity);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    if (rc != SQLITE_DONE) {
        return -6;
    }

    // Build redirect: /profile/<encoded username>
    const char* prefix = "/profile/";
    size_t prefix_len = strlen(prefix);
    if (out_redirect_size < prefix_len + 1) return -7;
    memcpy(out_redirect, prefix, prefix_len);
    if (!url_encode(username, out_redirect + prefix_len, out_redirect_size - prefix_len)) {
        return -8;
    }
    return 0;
}

int main(void) {
    // 5 test cases
    struct {
        const char* user;
        const char* stock;
        int qty;
    } tests[] = {
        {"alice", "AAPL", 10},
        {"bob-01", "Microsoft Corp", 250},
        {"charlie", "GOOG", 0},        // invalid quantity
        {"dave", "Bad'Stock", 5},      // invalid stock name
        {"eve", "TSLA", 100}
    };

    char redirect[256];
    for (int i = 0; i < 5; ++i) {
        int rc = place_buy_order(tests[i].user, tests[i].stock, tests[i].qty, redirect, sizeof(redirect));
        if (rc == 0) {
            printf("Test%d: %s\n", i + 1, redirect);
        } else {
            printf("Test%d: INVALID (code %d)\n", i + 1, rc);
        }
    }
    return 0;
}