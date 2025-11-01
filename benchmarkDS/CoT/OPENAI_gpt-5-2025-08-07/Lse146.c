#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sqlite3.h"

static int is_valid_username(const char* s) {
    if (!s) return 0;
    size_t len = 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        unsigned char c = *p;
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == '_' || c == '.' || c == '-')) {
            return 0;
        }
        if (++len > 64) return 0;
    }
    return len >= 1;
}

static int is_valid_stock_name(const char* s) {
    if (!s) return 0;
    size_t len = 0;
    for (const unsigned char* p = (const unsigned char*)s; *p; ++p) {
        unsigned char c = *p;
        if (!((c >= 'A' && c <= 'Z') ||
              (c >= 'a' && c <= 'z') ||
              (c >= '0' && c <= '9') ||
              c == ' ' || c == '_' || c == '.' || c == '-')) {
            return 0;
        }
        if (++len > 128) return 0;
    }
    return len >= 1;
}

static char* url_encode(const char* s) {
    if (!s) return NULL;
    const char* hex = "0123456789ABCDEF";
    size_t len = strlen(s);
    size_t max_len = len * 3 + 1;
    char* out = (char*)malloc(max_len);
    if (!out) return NULL;
    size_t j = 0;
    for (size_t i = 0; i < len; ++i) {
        unsigned char c = (unsigned char)s[i];
        if ((c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out[j++] = (char)c;
        } else {
            out[j++] = '%';
            out[j++] = hex[(c >> 4) & 0xF];
            out[j++] = hex[c & 0xF];
        }
    }
    out[j] = '\0';
    return out;
}

static void iso8601_now_utc(char* buf, size_t bufsize) {
    time_t t = time(NULL);
    struct tm tm;
#if defined(_WIN32)
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    strftime(buf, bufsize, "%Y-%m-%dT%H:%M:%SZ", &tm);
}

char* place_buy_order(const char* username, const char* stock_name, int stock_quantity) {
    if (!is_valid_username(username)) { return NULL; }
    if (!is_valid_stock_name(stock_name)) { return NULL; }
    if (stock_quantity <= 0 || stock_quantity > 1000000) { return NULL; }

    sqlite3* db = NULL;
    if (sqlite3_open("orders_c.db", &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return NULL;
    }

    const char* create_sql =
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "stock_name TEXT NOT NULL,"
        "quantity INTEGER NOT NULL,"
        "created_at TEXT NOT NULL"
        ");";
    char* errMsg = NULL;
    if (sqlite3_exec(db, create_sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return NULL;
    }

    const char* insert_sql = "INSERT INTO buy_orders (username, stock_name, quantity, created_at) VALUES (?, ?, ?, ?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return NULL;
    }

    char ts[32];
    iso8601_now_utc(ts, sizeof(ts));

    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, stock_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, stock_quantity);
    sqlite3_bind_text(stmt, 4, ts, -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    char* enc = url_encode(username);
    if (!enc) return NULL;
    const char* prefix = "/profile/";
    size_t out_len = strlen(prefix) + strlen(enc) + 1;
    char* result = (char*)malloc(out_len);
    if (!result) {
        free(enc);
        return NULL;
    }
    strcpy(result, prefix);
    strcat(result, enc);
    free(enc);
    return result;
}

int main(void) {
    struct Test { const char* u; const char* s; int q; } tests[5] = {
        {"alice", "AAPL", 10},
        {"bob.smith", "GOOGL", 25},
        {"charlie-d", "MSFT", 1000},
        {"diana", "Tesla Inc", 15},
        {"eve_99", "AMZN", 5}
    };
    for (int i = 0; i < 5; ++i) {
        char* redirect = place_buy_order(tests[i].u, tests[i].s, tests[i].q);
        if (redirect) {
            printf("Redirect to: %s\n", redirect);
            free(redirect);
        } else {
            printf("Error placing order for %s\n", tests[i].u);
        }
    }
    return 0;
}