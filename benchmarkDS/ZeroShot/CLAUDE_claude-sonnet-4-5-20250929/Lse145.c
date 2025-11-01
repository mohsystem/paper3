
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>

#define DB_NAME "stocks.db"
#define MAX_STOCK_NAME 100

// Function to sanitize stock name
void sanitize_stock_name(char* dest, const char* src, size_t max_len) {
    size_t j = 0;
    for (size_t i = 0; src[i] != '\\0' && j < max_len - 1; i++) {
        if (isalnum(src[i])) {
            dest[j++] = src[i];
        }
    }
    dest[j] = '\\0';
}

// Initialize database
void init_database() {
    sqlite3* db;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database initialization error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS stocks ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                     "stock_name TEXT NOT NULL, "
                     "quantity INTEGER NOT NULL, "
                     "purchase_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

// Helper function
int buy_function(const char* stock_name) {
    if (stock_name == NULL || strlen(stock_name) == 0) {
        printf("Invalid stock name\\n");
        return 0;
    }
    
    printf("Processing purchase for stock: %s\\n", stock_name);
    return 1;
}

// Main buy stock function with security measures
int buy_stock(const char* stock_name, int quantity) {
    char sanitized_name[MAX_STOCK_NAME];
    
    // Input validation
    if (stock_name == NULL || strlen(stock_name) == 0) {
        printf("Error: Stock name cannot be empty\\n");
        return 0;
    }
    
    if (quantity <= 0) {
        printf("Error: Quantity must be positive\\n");
        return 0;
    }
    
    // Sanitize input
    sanitize_stock_name(sanitized_name, stock_name, MAX_STOCK_NAME);
    
    if (strlen(sanitized_name) == 0) {
        printf("Error: Invalid stock name after sanitization\\n");
        return 0;
    }
    
    // Call buy_function
    if (!buy_function(sanitized_name)) {
        return 0;
    }
    
    // Insert into database using parameterized query
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database error: %s\\n", sqlite3_errmsg(db));
        return 0;
    }
    
    const char* sql = "INSERT INTO stocks(stock_name, quantity) VALUES(?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, sanitized_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_DONE) {
        printf("Successfully purchased %d shares of %s\\n", quantity, sanitized_name);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }
    
    fprintf(stderr, "Execution error: %s\\n", sqlite3_errmsg(db));
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}

int main() {
    init_database();
    
    printf("=== Test Case 1: Valid purchase ===\\n");
    buy_stock("AAPL", 10);
    
    printf("\\n=== Test Case 2: Another valid purchase ===\\n");
    buy_stock("GOOGL", 5);
    
    printf("\\n=== Test Case 3: Invalid quantity (negative) ===\\n");
    buy_stock("MSFT", -5);
    
    printf("\\n=== Test Case 4: Invalid quantity (zero) ===\\n");
    buy_stock("TSLA", 0);
    
    printf("\\n=== Test Case 5: Empty stock name ===\\n");
    buy_stock("", 10);
    
    return 0;
}
