
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <stdbool.h>

#define DB_NAME "stocks.db"

// The buy_function that processes stock name
void buy_function(const char* stockName) {
    printf("Processing stock: %s\\n", stockName);
    // Additional stock processing logic can be added here
    printf("Stock %s processed successfully\\n", stockName);
}

// The buy_stock function that handles the purchase
bool buy_stock(const char* stockName, int quantity) {
    sqlite3* db = NULL;
    char* errMsg = NULL;
    sqlite3_stmt* stmt = NULL;
    
    // Open database connection
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Error opening database: %s\\n", sqlite3_errmsg(db));
        return false;
    }
    
    // Create table if not exists
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS stocks ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "stock_name TEXT, "
        "quantity INTEGER, "
        "purchase_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
    
    rc = sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error creating table: %s\\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    // Insert stock data
    const char* insertSQL = "INSERT INTO stocks (stock_name, quantity) VALUES (?, ?)";
    
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Error preparing statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, stockName, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);
    
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Error inserting data: %s\\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
    
    printf("Stock inserted into database: %s, Quantity: %d\\n", stockName, quantity);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    // Call buy_function with stock name
    buy_function(stockName);
    
    return true;
}

int main() {
    printf("=== Test Case 1 ===\\n");
    buy_stock("AAPL", 10);
    
    printf("\\n=== Test Case 2 ===\\n");
    buy_stock("GOOGL", 5);
    
    printf("\\n=== Test Case 3 ===\\n");
    buy_stock("MSFT", 15);
    
    printf("\\n=== Test Case 4 ===\\n");
    buy_stock("TSLA", 8);
    
    printf("\\n=== Test Case 5 ===\\n");
    buy_stock("AMZN", 3);
    
    return 0;
}
