/* Chain-of-Through process in code generation for C:
   1. Problem understanding: Insert a (stock_name, quantity) into DB upon "buy_order", then show "stock_view".
   2. Security requirements: Validate inputs, use prepared statements, manage resources safely.
   3. Secure coding generation: Implement sqlite3 prepared statements, input checks, bounds.
   4. Code review: Ensure finalize/close on all statements/DB, check rc codes, avoid buffer overflows.
   5. Secure code output: Final implementation with 5 test cases.

   Compile with: cc this_file.c -lsqlite3
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>

static int is_valid_stock_name(const char* name) {
    if (name == NULL) return 0;
    size_t len = 0;
    for (const char* p = name; *p; ++p) {
        unsigned char c = (unsigned char)*p;
        if (!(isalnum(c) || c == '_' || c == '.' || c == '-')) return 0;
        len++;
        if (len > 64) return 0;
    }
    return len >= 1;
}

static int init_db(const char* path) {
    sqlite3* db = NULL;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 1;
    }
    const char* sql =
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        " id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " stock_name TEXT NOT NULL,"
        " quantity INTEGER NOT NULL CHECK(quantity > 0),"
        " created_at TEXT DEFAULT (datetime('now'))"
        ");";
    char* err = NULL;
    int rc = sqlite3_exec(db, sql, NULL, NULL, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
        sqlite3_close(db);
        return 2;
    }
    sqlite3_close(db);
    return 0;
}

/* Build a string representing the stock view. Caller must free the returned string. */
static char* stock_view(const char* path) {
    sqlite3* db = NULL;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return NULL;
    }
    const char* sql = "SELECT id, stock_name, quantity, created_at FROM buy_orders ORDER BY id ASC";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return NULL;
    }

    size_t cap = 1024;
    size_t len = 0;
    char* out = (char*)malloc(cap);
    if (!out) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }
    out[0] = '\0';

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* name = sqlite3_column_text(stmt, 1);
        int qty = sqlite3_column_int(stmt, 2);
        const unsigned char* created = sqlite3_column_text(stmt, 3);

        char line[512];
        snprintf(line, sizeof(line), "%d|%s|%d|%s\n", id,
                 name ? (const char*)name : "",
                 qty,
                 created ? (const char*)created : "");

        size_t need = strlen(line);
        if (len + need + 1 > cap) {
            size_t new_cap = (cap * 2 > len + need + 1) ? cap * 2 : (len + need + 1);
            char* tmp = (char*)realloc(out, new_cap);
            if (!tmp) {
                free(out);
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return NULL;
            }
            out = tmp;
            cap = new_cap;
        }
        memcpy(out + len, line, need);
        len += need;
        out[len] = '\0';
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return out;
}

/* Insert and return current stock view string (caller must free). */
static char* buy_order(const char* path, const char* stock_name, int quantity) {
    if (!is_valid_stock_name(stock_name)) {
        return NULL;
    }
    if (quantity <= 0 || quantity > 1000000) {
        return NULL;
    }

    sqlite3* db = NULL;
    if (sqlite3_open(path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return NULL;
    }
    const char* sql = "INSERT INTO buy_orders (stock_name, quantity) VALUES (?, ?)";
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        sqlite3_close(db);
        return NULL;
    }
    sqlite3_bind_text(stmt, 1, stock_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return NULL;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return stock_view(path);
}

int main(void) {
    const char* db = "c_demo.db";
    if (init_db(db) != 0) {
        fprintf(stderr, "DB init failed\n");
        return 1;
    }

    char* v1 = buy_order(db, "AAPL", 10);
    printf("After AAPL:\n%s", v1 ? v1 : "(error)\n");
    free(v1);

    char* v2 = buy_order(db, "GOOGL", 5);
    printf("After GOOGL:\n%s", v2 ? v2 : "(error)\n");
    free(v2);

    char* v3 = buy_order(db, "MSFT", 15);
    printf("After MSFT:\n%s", v3 ? v3 : "(error)\n");
    free(v3);

    char* v4 = buy_order(db, "TSLA", 1);
    printf("After TSLA:\n%s", v4 ? v4 : "(error)\n");
    free(v4);

    char* v5 = buy_order(db, "NVDA", 20);
    printf("After NVDA:\n%s", v5 ? v5 : "(error)\n");
    free(v5);

    return 0;
}