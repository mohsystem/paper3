#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

// This is a simulation. No actual database connection is made.
// The code demonstrates the secure use of a prepared statement API (like SQLite3's)
// to prevent SQL injection. In a real C application, you would link against a
// database library (e.g., libsqlite3, libpq, libmysqlclient).

// --- Helper Functions for Validation ---
// Returns 1 if valid, 0 otherwise.
int is_valid_username(const char* username) {
    if (username == NULL) return 0;
    int len = strlen(username);
    if (len < 3 || len > 20) return 0;
    for (int i = 0; i < len; i++) {
        if (!isalnum(username[i]) && username[i] != '_') {
            return 0;
        }
    }
    return 1;
}

// Returns 1 if valid, 0 otherwise.
int is_valid_stock_name(const char* stock_name) {
    if (stock_name == NULL) return 0;
    int len = strlen(stock_name);
    if (len == 0) return 0;
    for (int i = 0; i < len; i++) {
        if (!isalnum(stock_name[i]) && stock_name[i] != '.' && stock_name[i] != '-') {
            return 0;
        }
    }
    return 1;
}

/**
 * Securely inserts a buy order into the database using a prepared statement pattern.
 * This function simulates the API calls of a library like SQLite3.
 *
 * @param username The user placing the order.
 * @param stock_name The name of the stock.
 * @param stock_quantity The quantity of the stock to buy.
 * @return A status code (0 for success, 1 for error).
 */
int buyStock(const char* username, const char* stock_name, int stock_quantity) {
    char result_message[256];

    // 1. Input Validation
    if (!is_valid_username(username)) {
        snprintf(result_message, sizeof(result_message), "Error: Invalid username format.");
        printf("%s\n", result_message);
        return 1;
    }
    if (!is_valid_stock_name(stock_name)) {
        snprintf(result_message, sizeof(result_message), "Error: Invalid stock name format.");
        printf("%s\n", result_message);
        return 1;
    }
    if (stock_quantity <= 0) {
        snprintf(result_message, sizeof(result_message), "Error: Stock quantity must be a positive number.");
        printf("%s\n", result_message);
        return 1;
    }

    // 2. Use a prepared statement for security
    const char* sql = "INSERT INTO buy_orders (username, stock_name, quantity, order_date) VALUES (?, ?, ?, datetime('now'));";

    printf("\n--- New Order ---\n");
    printf("Executing secure buy order...\n");
    printf("Username: %s\n", username);
    printf("Stock: %s\n", stock_name);
    printf("Quantity: %d\n", stock_quantity);

    // In a real application, you would use a library's API.
    // Example with the SQLite3 C API:
    //
    // sqlite3* db;
    // sqlite3_stmt* stmt;
    // sqlite3_open("stocks.db", &db);
    // int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    // if (rc == SQLITE_OK) {
    //     sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    //     sqlite3_bind_text(stmt, 2, stock_name, -1, SQLITE_STATIC);
    //     sqlite3_bind_int(stmt, 3, stock_quantity);
    //     if (sqlite3_step(stmt) != SQLITE_DONE) {
    //         // Handle error
    //     }
    // }
    // sqlite3_finalize(stmt);
    // sqlite3_close(db);

    // Simulation of the prepared statement process
    printf("Simulating PreparedStatement...\n");
    printf("SQL Template: %s\n", sql);
    printf("Calling sqlite3_prepare_v2()...\n");
    printf("Calling sqlite3_bind_text(stmt, 1, \"%s\")...\n", username);
    printf("Calling sqlite3_bind_text(stmt, 2, \"%s\")...\n", stock_name);
    printf("Calling sqlite3_bind_int(stmt, 3, %d)...\n", stock_quantity);
    printf("Calling sqlite3_step() and sqlite3_finalize()...\n");
    printf("Simulation successful.\n");

    snprintf(result_message, sizeof(result_message), "Success: Order placed for %s. Redirecting to profile page.", username);
    printf("%s\n", result_message);
    
    return 0; // Success
}

int main() {
    // Test Case 1: Normal valid transaction
    buyStock("jdoe", "GOOGL", 10);

    // Test Case 2: Another valid transaction
    buyStock("asmith", "TSLA", 5);

    // Test Case 3: SQL Injection attempt (should be handled safely as a literal string)
    buyStock("evil_user", "FAKE.STK'); DROP TABLE buy_orders; --", 1);

    // Test Case 4: Invalid quantity
    buyStock("jdoe", "MSFT", -50);
    
    // Test Case 5: Invalid username with spaces
    buyStock("bad user", "AAPL", 100);

    return 0;
}