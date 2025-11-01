
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

int buyStock(const char* username, const char* stockName, int stockQuantity) {
    sqlite3* db;
    char* errMsg = NULL;
    int rc;
    
    /* Open database */
    rc = sqlite3_open("stock_db.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    /* Create table if not exists */
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "stock_name TEXT NOT NULL,"
        "stock_quantity INTEGER NOT NULL,"
        "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    
    rc = sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 0;
    }
    
    /* Prepare insert statement */
    sqlite3_stmt* stmt;
    const char* sql = "INSERT INTO buy_orders (username, stock_name, stock_quantity) VALUES (?, ?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    /* Bind parameters */
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, stockName, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, stockQuantity);
    
    /* Execute statement */
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
    
    printf("Buy order inserted successfully!\\n");
    printf("Redirecting to profile page...\\n");
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 1;
}

int main() {
    /* Test cases */
    printf("Test Case 1:\\n");
    buyStock("john_doe", "AAPL", 10);
    
    printf("\\nTest Case 2:\\n");
    buyStock("jane_smith", "GOOGL", 5);
    
    printf("\\nTest Case 3:\\n");
    buyStock("bob_jones", "MSFT", 15);
    
    printf("\\nTest Case 4:\\n");
    buyStock("alice_williams", "TSLA", 20);
    
    printf("\\nTest Case 5:\\n");
    buyStock("charlie_brown", "AMZN", 8);
    
    return 0;
}
