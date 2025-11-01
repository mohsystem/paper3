
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdbool.h>

#define DB_NAME "stocks.db"
#define MAX_STOCK_NAME 100

void initializeDatabase() {
    sqlite3* db;
    char* errMsg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Error opening database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "stock_name TEXT NOT NULL, "
        "quantity INTEGER NOT NULL, "
        "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
    
    rc = sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error creating table: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

bool buyOrder(const char* stockName, int quantity) {
    if (stockName == NULL || strlen(stockName) == 0 || quantity <= 0) {
        return false;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Error opening database: %s\\n", sqlite3_errmsg(db));
        return false;
    }
    
    const char* insertSQL = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?)";
    
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, stockName, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);
    
    rc = sqlite3_step(stmt);
    bool success = (rc == SQLITE_DONE);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

void stockView() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Error opening database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* selectSQL = "SELECT * FROM orders ORDER BY order_date DESC";
    
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* stockName = sqlite3_column_text(stmt, 1);
        int quantity = sqlite3_column_int(stmt, 2);
        const unsigned char* orderDate = sqlite3_column_text(stmt, 3);
        
        printf("ID: %d, Stock: %s, Quantity: %d, Date: %s\\n", 
               id, stockName, quantity, orderDate);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main() {
    initializeDatabase();
    
    printf("Test 1: Adding AAPL with quantity 10\\n");
    bool result1 = buyOrder("AAPL", 10);
    printf("Result: %s\\n", result1 ? "Success" : "Failed");
    
    printf("\\nTest 2: Adding GOOGL with quantity 5\\n");
    bool result2 = buyOrder("GOOGL", 5);
    printf("Result: %s\\n", result2 ? "Success" : "Failed");
    
    printf("\\nTest 3: Adding empty stock name with quantity 15\\n");
    bool result3 = buyOrder("", 15);
    printf("Result: %s\\n", result3 ? "Success" : "Failed");
    
    printf("\\nTest 4: Adding MSFT with quantity -5\\n");
    bool result4 = buyOrder("MSFT", -5);
    printf("Result: %s\\n", result4 ? "Success" : "Failed");
    
    printf("\\nTest 5: Adding TSLA with quantity 20\\n");
    bool result5 = buyOrder("TSLA", 20);
    printf("Result: %s\\n", result5 ? "Success" : "Failed");
    
    printf("\\n=== Stock View ===\\n");
    stockView();
    
    return 0;
}
