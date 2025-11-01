
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <regex.h>
#include <stdbool.h>

#define MAX_USERNAME_LEN 30
#define MAX_STOCK_NAME_LEN 20
#define MAX_RESULT_LEN 256

bool validate_username(const char* username) {
    if (username == NULL || strlen(username) < 3 || strlen(username) > 30) {
        return false;
    }
    
    regex_t regex;
    int reti = regcomp(&regex, "^[a-zA-Z0-9_]{3,30}$", REG_EXTENDED);
    if (reti != 0) return false;
    
    reti = regexec(&regex, username, 0, NULL, 0);
    regfree(&regex);
    
    return reti == 0;
}

bool validate_stock_name(const char* stock_name) {
    if (stock_name == NULL || strlen(stock_name) < 1 || strlen(stock_name) > 20) {
        return false;
    }
    
    regex_t regex;
    int reti = regcomp(&regex, "^[a-zA-Z0-9_.-]{1,20}$", REG_EXTENDED);
    if (reti != 0) return false;
    
    reti = regexec(&regex, stock_name, 0, NULL, 0);
    regfree(&regex);
    
    return reti == 0;
}

bool validate_quantity(int quantity) {
    return quantity > 0 && quantity <= 1000000;
}

void insert_buy_order(const char* username, const char* stock_name, 
                     int stock_quantity, char* result) {
    if (!validate_username(username)) {
        strcpy(result, "Error: Invalid username format");
        return;
    }
    if (!validate_stock_name(stock_name)) {
        strcpy(result, "Error: Invalid stock name format");
        return;
    }
    if (!validate_quantity(stock_quantity)) {
        strcpy(result, "Error: Invalid quantity (must be 1-1000000)");
        return;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc = sqlite3_open("stock_orders.db", &db);
    
    if (rc != SQLITE_OK) {
        snprintf(result, MAX_RESULT_LEN, "Error: Cannot open database - %s", 
                sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    const char* insert_sql = 
        "INSERT INTO buy_orders (username, stock_name, quantity, order_date) "
        "VALUES (?, ?, ?, datetime('now'))";
    
    rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        snprintf(result, MAX_RESULT_LEN, "Error: Failed to prepare statement - %s", 
                sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, stock_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, stock_quantity);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_DONE) {
        snprintf(result, MAX_RESULT_LEN, "redirect:/profile?user=%s", username);
    } else {
        snprintf(result, MAX_RESULT_LEN, "Error: Failed to insert order - %s", 
                sqlite3_errmsg(db));
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

int main() {
    char result[MAX_RESULT_LEN];
    
    printf("=== Stock Order System Test Cases ===\\n\\n");
    
    // Test Case 1: Valid order
    printf("Test 1 - Valid order:\\n");
    insert_buy_order("john_doe", "AAPL", 100, result);
    printf("%s\\n\\n", result);
    
    // Test Case 2: Another valid order
    printf("Test 2 - Valid order with different stock:\\n");
    insert_buy_order("jane_smith", "GOOGL", 50, result);
    printf("%s\\n\\n", result);
    
    // Test Case 3: Invalid username
    printf("Test 3 - Invalid username:\\n");
    insert_buy_order("user@123", "MSFT", 75, result);
    printf("%s\\n\\n", result);
    
    // Test Case 4: Invalid stock name
    printf("Test 4 - Invalid stock name:\\n");
    insert_buy_order("testuser", "STOCK'; DROP TABLE--", 25, result);
    printf("%s\\n\\n", result);
    
    // Test Case 5: Invalid quantity
    printf("Test 5 - Invalid quantity:\\n");
    insert_buy_order("alice", "TSLA", -10, result);
    printf("%s\\n", result);
    
    return 0;
}
