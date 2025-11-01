
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <stdbool.h>

void trim(char* str) {
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == 0) {
        *str = 0;
        return;
    }
    
    char* end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    size_t len = end - start + 1;
    memmove(str, start, len);
    str[len] = 0;
}

void sanitize_stock_name(char* str) {
    char* src = str;
    char* dst = str;
    
    while (*src) {
        if (isalnum((unsigned char)*src)) {
            *dst++ = *src;
        }
        src++;
    }
    *dst = 0;
}

bool process_buy_order(const char* username, const char* stock_name, int stock_quantity) {
    // Input validation
    if (username == NULL || strlen(username) == 0) {
        printf("Error: Username cannot be empty\\n");
        return false;
    }
    
    if (stock_name == NULL || strlen(stock_name) == 0) {
        printf("Error: Stock name cannot be empty\\n");
        return false;
    }
    
    if (stock_quantity <= 0) {
        printf("Error: Stock quantity must be positive\\n");
        return false;
    }
    
    // Sanitize inputs
    char clean_username[256];
    char clean_stock_name[256];
    strncpy(clean_username, username, sizeof(clean_username) - 1);
    strncpy(clean_stock_name, stock_name, sizeof(clean_stock_name) - 1);
    clean_username[sizeof(clean_username) - 1] = '\\0';
    clean_stock_name[sizeof(clean_stock_name) - 1] = '\\0';
    
    trim(clean_username);
    trim(clean_stock_name);
    sanitize_stock_name(clean_stock_name);
    
    if (strlen(clean_username) == 0 || strlen(clean_stock_name) == 0) {
        printf("Error: Invalid input after sanitization\\n");
        return false;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    char* err_msg = NULL;
    
    // Open database
    int rc = sqlite3_open("trading.db", &db);
    if (rc != SQLITE_OK) {
        printf("Database error: %s\\n", sqlite3_errmsg(db));
        return false;
    }
    
    // Create table if not exists
    const char* create_table_sql = 
        "CREATE TABLE IF NOT EXISTS buy_orders ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "username TEXT NOT NULL,"
        "stock_name TEXT NOT NULL,"
        "stock_quantity INTEGER NOT NULL,"
        "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
    
    rc = sqlite3_exec(db, create_table_sql, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return false;
    }
    
    // Use parameterized query to prevent SQL injection
    const char* sql = "INSERT INTO buy_orders (username, stock_name, stock_quantity) VALUES (?, ?, ?)";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        printf("SQL error: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, clean_username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, clean_stock_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, stock_quantity);
    
    rc = sqlite3_step(stmt);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    if (rc == SQLITE_DONE) {
        printf("Buy order placed successfully!\\n");
        printf("Redirecting to profile page...\\n");
        return true;
    }
    
    return false;
}

int main() {
    printf("Testing Buy Order Processing System\\n\\n");
    
    // Test case 1: Valid order
    printf("Test 1: Valid buy order\\n");
    process_buy_order("john_doe", "AAPL", 100);
    printf("\\n");
    
    // Test case 2: Another valid order
    printf("Test 2: Valid buy order for different stock\\n");
    process_buy_order("jane_smith", "GOOGL", 50);
    printf("\\n");
    
    // Test case 3: Invalid - empty username
    printf("Test 3: Empty username (should fail)\\n");
    process_buy_order("", "TSLA", 75);
    printf("\\n");
    
    // Test case 4: Invalid - negative quantity
    printf("Test 4: Negative quantity (should fail)\\n");
    process_buy_order("bob_jones", "MSFT", -10);
    printf("\\n");
    
    // Test case 5: Invalid - empty stock name
    printf("Test 5: Empty stock name (should fail)\\n");
    process_buy_order("alice_wilson", "", 200);
    
    return 0;
}
