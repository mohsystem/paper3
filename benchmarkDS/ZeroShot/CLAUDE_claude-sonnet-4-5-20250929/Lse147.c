
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>

#define DB_NAME "stocks.db"
#define MAX_STOCK_NAME 50

// Function to trim whitespace
void trim(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\\0';
}

// Function to validate stock name format
int isValidStockName(const char* stockName) {
    if (stockName == NULL || strlen(stockName) == 0) {
        return 0;
    }
    
    for (int i = 0; stockName[i] != '\\0'; i++) {
        char c = stockName[i];
        if (!isalnum(c) && c != '.' && c != '_' && c != '-') {
            return 0;
        }
    }
    return 1;
}

// Initialize database and create table if not exists
void initializeDatabase() {
    sqlite3* db;
    char* errMsg = NULL;
    int rc;
    
    rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS orders ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "stock_name TEXT NOT NULL, "
                     "quantity INTEGER NOT NULL, "
                     "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

// Secure method to insert buy order using prepared statements
int buyOrder(const char* stockName, int quantity) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    char cleanStockName[MAX_STOCK_NAME];
    
    // Input validation
    if (stockName == NULL || strlen(stockName) == 0) {
        fprintf(stderr, "Error: Stock name cannot be empty\\n");
        return 0;
    }
    
    if (quantity <= 0) {
        fprintf(stderr, "Error: Quantity must be positive\\n");
        return 0;
    }
    
    // Copy and trim stock name
    strncpy(cleanStockName, stockName, MAX_STOCK_NAME - 1);
    cleanStockName[MAX_STOCK_NAME - 1] = '\\0';
    trim(cleanStockName);
    
    // Validate stock name format
    if (!isValidStockName(cleanStockName)) {
        fprintf(stderr, "Error: Invalid stock name format\\n");
        return 0;
    }
    
    rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    const char* sql = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    // Bind parameters
    sqlite3_bind_text(stmt, 1, cleanStockName, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
    
    printf("Order placed successfully!\\n");
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 1;
}

// Method to view all stock orders
void stockView() {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    
    rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* sql = "SELECT id, stock_name, quantity, order_date FROM orders ORDER BY order_date DESC";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    printf("\\n=== Stock Orders ===\\n");
    printf("%-5s %-15s %-10s %-20s\\n", "ID", "Stock Name", "Quantity", "Order Date");
    printf("-------------------------------------------------------\\n");
    
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        int id = sqlite3_column_int(stmt, 0);
        const char* stockName = (const char*)sqlite3_column_text(stmt, 1);
        int quantity = sqlite3_column_int(stmt, 2);
        const char* orderDate = (const char*)sqlite3_column_text(stmt, 3);
        
        printf("%-5d %-15s %-10d %-20s\\n", id, stockName, quantity, orderDate);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main() {
    initializeDatabase();
    
    // Test cases
    printf("Running test cases...\\n\\n");
    
    // Test case 1: Valid order
    printf("Test 1: Valid order\\n");
    buyOrder("AAPL", 100);
    
    // Test case 2: Valid order with different stock
    printf("\\nTest 2: Another valid order\\n");
    buyOrder("GOOGL", 50);
    
    // Test case 3: Invalid - empty stock name
    printf("\\nTest 3: Empty stock name\\n");
    buyOrder("", 25);
    
    // Test case 4: Invalid - negative quantity
    printf("\\nTest 4: Negative quantity\\n");
    buyOrder("MSFT", -10);
    
    // Test case 5: Invalid - special characters in stock name
    printf("\\nTest 5: Invalid stock name with SQL injection attempt\\n");
    buyOrder("TSLA'; DROP TABLE orders; --", 75);
    
    // Display all orders
    stockView();
    
    return 0;
}
