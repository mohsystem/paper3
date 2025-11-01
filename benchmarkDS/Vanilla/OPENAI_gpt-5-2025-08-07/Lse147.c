#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int init_db(const char* db_path) {
    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS orders ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  stock TEXT NOT NULL,"
        "  qty INTEGER NOT NULL,"
        "  created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ")";
    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

long long buy_order(const char* db_path, const char* stock, int qty) {
    if (!stock || strlen(stock) == 0) {
        fprintf(stderr, "stock is required\n");
        return -1;
    }
    if (qty <= 0) {
        fprintf(stderr, "qty must be positive\n");
        return -1;
    }

    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return -1;
    }

    const char* sql = "INSERT INTO orders(stock, qty) VALUES(?, ?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return -1;
    }

    sqlite3_bind_text(stmt, 1, stock, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, qty);

    long long rowid = -1;
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        rowid = sqlite3_last_insert_rowid(db);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return rowid;
}

static void append_str(char** buf, size_t* cap, size_t* len, const char* s) {
    size_t sl = strlen(s);
    if (*len + sl + 1 > *cap) {
        size_t newcap = (*cap == 0 ? 1024 : *cap);
        while (*len + sl + 1 > newcap) newcap *= 2;
        char* nb = (char*)realloc(*buf, newcap);
        if (!nb) return;
        *buf = nb;
        *cap = newcap;
    }
    memcpy(*buf + *len, s, sl);
    *len += sl;
    (*buf)[*len] = '\0';
}

char* stock_view(const char* db_path) {
    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return NULL;
    }

    const char* sql = "SELECT id, stock, qty, created_at FROM orders ORDER BY id";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return NULL;
    }

    char* out = NULL;
    size_t cap = 0, len = 0;
    char line[1024];

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        long long id = sqlite3_column_int64(stmt, 0);
        const unsigned char* stock = sqlite3_column_text(stmt, 1);
        int qty = sqlite3_column_int(stmt, 2);
        const unsigned char* created_at = sqlite3_column_text(stmt, 3);

        snprintf(line, sizeof(line), "%lld,%s,%d,%s\n",
                 id,
                 stock ? (const char*)stock : "",
                 qty,
                 created_at ? (const char*)created_at : "");
        append_str(&out, &cap, &len, line);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    if (!out) {
        out = (char*)malloc(1);
        if (out) out[0] = '\0';
    }
    return out;
}

int main(void) {
    const char* db = "stocks_c.db";
    if (!init_db(db)) return 1;

    buy_order(db, "AAPL", 10);
    char* v1 = stock_view(db); if (v1) { printf("%s", v1); free(v1); }

    buy_order(db, "GOOG", 5);
    char* v2 = stock_view(db); if (v2) { printf("%s", v2); free(v2); }

    buy_order(db, "MSFT", 20);
    char* v3 = stock_view(db); if (v3) { printf("%s", v3); free(v3); }

    buy_order(db, "TSLA", 3);
    char* v4 = stock_view(db); if (v4) { printf("%s", v4); free(v4); }

    buy_order(db, "AMZN", 7);
    char* v5 = stock_view(db); if (v5) { printf("%s", v5); free(v5); }

    return 0;
}