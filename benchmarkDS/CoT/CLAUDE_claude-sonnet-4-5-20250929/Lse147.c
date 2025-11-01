
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>

#define DB_NAME "stocks.db"
#define MAX_STOCK_NAME 51

int validateStockName(const char* stockName) {
    if (!stockName || strlen(stockName) == 0 || strlen(stockName) > 50) {
        return 0;
    }
    
    for (int i = 0; stockName[i]; i++) {
        if (!isalnum(stockName[i]) && stockName[i] != '_' && stockName[i] != '-') {
            return 0;
        }
    }
    return 1;
}

int validateQuantity(int quantity) {
    return quantity > 0;
}

void initializeDatabase() {
    sqlite3* db;
    char* errMsg = NULL;
    
    if (sqlite3_open(DB_NAME, &db) == SQLITE_OK) {
        const char* sql = "CREATE TABLE IF NOT EXISTS orders ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "stock_name TEXT NOT NULL,"
                         "quantity INTEGER NOT NULL,"
                         "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
        
        sqlite3_exec(db, sql, NULL, NULL, &errMsg);
        if (errMsg) {
            sqlite3_free(errMsg);
        }
    }
    sqlite3_close(db);
}

int buyOrder(const char* stockName, int quantity) {
    // Input validation
    if (!validateStockName(stockName)) {
        printf("Error: Invalid stock name format\\n");
        return 0;
    }
    
    if (!validateQuantity(quantity)) {
        printf("Error: Quantity must be positive\\n");
        return 0;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        printf("Error opening database\\n");
        return 0;
    }
    
    // Parameterized query prevents SQL injection
    const char* sql = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, stockName, -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, quantity);
        
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            printf("Order placed successfully: %s x %d\\n", stockName, quantity);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return 1;
        }
    }
    
    printf("Error placing order\\n");
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

void stockView() {
    printf("\\n=== Stock View ===\\n");
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        printf("Error opening database\\n");
        return;
    }
    
    const char* sql = "SELECT id, stock_name, quantity, order_date FROM orders";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("ID: %d, Stock: %s, Quantity: %d, Date: %s\\n",
                   sqlite3_column_int(stmt, 0),
                   sqlite3_column_text(stmt, 1),
                   sqlite3_column_int(stmt, 2),
                   sqlite3_column_text(stmt, 3));
        }
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
}

int main() {
    // Initialize database
    initializeDatabase();
    
    // Test cases
    printf("Test Case 1:\\n");
    buyOrder("AAPL", 10);
    
    printf("\\nTest Case 2:\\n");
    buyOrder("GOOGL", 5);
    
    printf("\\nTest Case 3:\\n");
    buyOrder("MSFT", 15);
    
    printf("\\nTest Case 4 (Invalid - negative quantity):\\n");
    buyOrder("TSLA", -5);
    
    printf("\\nTest Case 5 (Invalid - SQL injection attempt):\\n");
    buyOrder("AAPL'; DROP TABLE orders;--", 10);
    
    // Display all orders
    stockView();
    
    return 0;
}
