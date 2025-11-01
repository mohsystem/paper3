#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "sqlite3.h"

static char* trim_copy(const char* s) {
    if (s == NULL) {
        char* empty = (char*)malloc(1);
        if (empty) empty[0] = '\0';
        return empty;
    }
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

static int is_stock_name_allowed_trimmed(const char* name) {
    if (!name) return 0;
    size_t len = strlen(name);
    if (len == 0 || len > 50) return 0;
    for (size_t i = 0; i < len; i++) {
        unsigned char ch = (unsigned char)name[i];
        if (!(isalnum(ch) || ch == ' ' || ch == '.' || ch == '_' || ch == '-')) {
            return 0;
        }
    }
    return 1;
}

int init_db(const char* db_path) {
    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        if (db) sqlite3_close(db);
        return 0;
    }
    const char* ddl =
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "stock_name TEXT NOT NULL,"
        "quantity INTEGER NOT NULL,"
        "created_at TEXT DEFAULT CURRENT_TIMESTAMP"
        ");";
    char* errMsg = NULL;
    int rc = sqlite3_exec(db, ddl, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return 0;
    }
    sqlite3_close(db);
    return 1;
}

// Simulates the buy_order route: validates input, inserts into DB securely, returns redirect path or error message.
char* buy_order(const char* db_path, const char* stock_name_in, int quantity) {
    char* trimmed = trim_copy(stock_name_in);
    if (!trimmed) {
        return strdup("ERROR: Memory failure");
    }
    if (!is_stock_name_allowed_trimmed(trimmed)) {
        free(trimmed);
        return strdup("ERROR: Invalid stock name");
    }
    if (quantity <= 0 || quantity > 1000000) {
        free(trimmed);
        return strdup("ERROR: Invalid quantity");
    }

    sqlite3* db = NULL;
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        free(trimmed);
        if (db) sqlite3_close(db);
        return strdup("ERROR: Database failure");
    }

    char* errMsg = NULL;
    if (sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", NULL, NULL, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        free(trimmed);
        return strdup("ERROR: Database failure");
    }

    const char* sql = "INSERT INTO buy_orders (stock_name, quantity) VALUES (?, ?)";
    sqlite3_stmt* stmt = NULL;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_close(db);
        free(trimmed);
        return strdup("ERROR: Database failure");
    }

    sqlite3_bind_text(stmt, 1, trimmed, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);

    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    free(trimmed);

    if (rc != SQLITE_DONE) {
        sqlite3_exec(db, "ROLLBACK;", NULL, NULL, NULL);
        sqlite3_close(db);
        return strdup("ERROR: Database failure");
    }

    if (sqlite3_exec(db, "COMMIT;", NULL, NULL, &errMsg) != SQLITE_OK) {
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return strdup("ERROR: Database failure");
    }

    sqlite3_close(db);
    return strdup("/stock_view");
}

int main(void) {
    const char* dbp = "c_buy_orders.db";
    printf("Init DB: %s\n", init_db(dbp) ? "OK" : "FAIL");

    // 5 test cases
    char* r1 = buy_order(dbp, "AAPL", 10);                        printf("%s\n", r1); free(r1);
    char* r2 = buy_order(dbp, "MSFT-2025", 1);                    printf("%s\n", r2); free(r2);
    char* r3 = buy_order(dbp, "AAPL; DROP TABLE buy_orders;", 5); printf("%s\n", r3); free(r3);
    char* r4 = buy_order(dbp, "GOOG", 0);                         printf("%s\n", r4); free(r4);
    char* longname = (char*)malloc(52);
    memset(longname, 'A', 51); longname[51] = '\0';
    char* r5 = buy_order(dbp, longname, 2);                       printf("%s\n", r5); free(r5);
    free(longname);
    return 0;
}