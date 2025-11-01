#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>

// Step 1-5: Securely validate inputs and use parameterized SQL to prevent injection.

static sqlite3* g_db = NULL;

static void ensure_db(void) {
    if (g_db == NULL) {
        if (sqlite3_open(":memory:", &g_db) != SQLITE_OK) {
            fprintf(stderr, "Failed to open database.\n");
            exit(1);
        }
        const char* ddl =
            "CREATE TABLE IF NOT EXISTS purchases ("
            "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  stock_name TEXT NOT NULL,"
            "  quantity INTEGER NOT NULL,"
            "  created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ");";
        char* err = NULL;
        if (sqlite3_exec(g_db, ddl, NULL, NULL, &err) != SQLITE_OK) {
            fprintf(stderr, "DDL error: %s\n", err ? err : "unknown");
            if (err) sqlite3_free(err);
            exit(1);
        }
    }
}

// buy_function: validates and returns newly allocated uppercase symbol (caller must free)
char* buy_function(const char* stock_name) {
    if (stock_name == NULL) {
        fprintf(stderr, "Stock name is required.\n");
        return NULL;
    }
    // Trim leading/trailing whitespace
    const char* start = stock_name;
    while (*start && isspace((unsigned char)*start)) start++;
    const char* end = stock_name + strlen(stock_name);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    size_t len = (size_t)(end - start);
    if (len == 0) {
        fprintf(stderr, "Stock name cannot be empty.\n");
        return NULL;
    }
    if (len > 10) {
        fprintf(stderr, "Stock name must be 1-10 alphabetic characters.\n");
        return NULL;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!isalpha((unsigned char)start[i])) {
            fprintf(stderr, "Stock name must be 1-10 alphabetic characters.\n");
            return NULL;
        }
    }
    char* out = (char*)malloc(len + 1);
    if (!out) return NULL;
    for (size_t i = 0; i < len; ++i) {
        out[i] = (char)toupper((unsigned char)start[i]);
    }
    out[len] = '\0';
    return out;
}

// buy_stock: validates quantity, calls buy_function, inserts, returns row id or -1 on error
long long buy_stock(const char* stock_name, int quantity) {
    if (quantity <= 0 || quantity > 1000000) {
        fprintf(stderr, "Quantity must be between 1 and 1,000,000.\n");
        return -1;
    }
    ensure_db();

    char* symbol = buy_function(stock_name);
    if (symbol == NULL) {
        return -1;
    }

    sqlite3_stmt* stmt = NULL;
    const char* sql = "INSERT INTO purchases (stock_name, quantity) VALUES (?, ?)";
    if (sqlite3_prepare_v2(g_db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement.\n");
        free(symbol);
        return -1;
    }

    if (sqlite3_bind_text(stmt, 1, symbol, -1, SQLITE_TRANSIENT) != SQLITE_OK) {
        fprintf(stderr, "Bind error (stock_name).\n");
        sqlite3_finalize(stmt);
        free(symbol);
        return -1;
    }
    if (sqlite3_bind_int(stmt, 2, quantity) != SQLITE_OK) {
        fprintf(stderr, "Bind error (quantity).\n");
        sqlite3_finalize(stmt);
        free(symbol);
        return -1;
    }

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Insert failed.\n");
        sqlite3_finalize(stmt);
        free(symbol);
        return -1;
    }
    sqlite3_finalize(stmt);
    free(symbol);

    return (long long)sqlite3_last_insert_rowid(g_db);
}

static void run_test(const char* label, const char* stock, int qty) {
    long long id = buy_stock(stock, qty);
    if (id >= 0) {
        printf("%s OK -> rowId=%lld\n", label, id);
    } else {
        printf("%s ERR\n", label);
    }
}

int main(void) {
    // 5 test cases
    run_test("T1", "AAPL", 10);    // valid
    run_test("T2", "goog", 5);     // valid (normalized)
    run_test("T3", "BAD!", 3);     // invalid name
    run_test("T4", "DROP", 0);     // invalid quantity
    run_test("T5", "msft", 100);   // valid
    return 0;
}