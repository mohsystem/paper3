#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "sqlite3.h"

// Compile with: gcc main.c -lsqlite3 -o main

const char* DB_FILE = "orders_c.db";
#define MAX_STOCK_NAME_LEN 10
#define MSG_BUFFER_SIZE 256

// Rule #1: Validate input
bool is_valid_stock_name(const char* name) {
    if (name == NULL) {
        return false;
    }
    size_t len = strnlen(name, MAX_STOCK_NAME_LEN + 1);
    if (len == 0 || len > MAX_STOCK_NAME_LEN) {
        return false;
    }
    for (size_t i = 0; i < len; ++i) {
        if (!((name[i] >= 'A' && name[i] <= 'Z') || name[i] == '.')) {
            return false;
        }
    }
    return true;
}

void setup_database() {
    sqlite3* db = NULL;
    char* err_msg = NULL;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        if (db) sqlite3_close(db);
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS orders("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "stock_name TEXT NOT NULL, "
                      "quantity INTEGER NOT NULL);";

    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
    
    sqlite3_close(db);
}

// Returns a dynamically allocated string. The caller must free it.
char* buy_order(const char* stock_name, int quantity) {
    char* result_msg = NULL;
    char buffer[MSG_BUFFER_SIZE];

    // Rule #1: Ensure all input is validated
    if (!is_valid_stock_name(stock_name)) {
        snprintf(buffer, sizeof(buffer), "Error: Invalid stock name format. Must be 1-%d uppercase letters/dots.", MAX_STOCK_NAME_LEN);
        result_msg = strdup(buffer);
        return result_msg;
    }
    if (quantity <= 0) {
        result_msg = strdup("Error: Quantity must be a positive integer.");
        return result_msg;
    }

    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc = 0;
    
    rc = sqlite3_open(DB_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        result_msg = strdup("Error: Could not process order due to a database error.");
        goto cleanup;
    }
    
    const char* sql = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?);";

    // Rule #2/#5: Use prepared statements
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        result_msg = strdup("Error: Could not process order due to a database error.");
        goto cleanup;
    }

    rc = sqlite3_bind_text(stmt, 1, stock_name, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind stock name: %s\n", sqlite3_errmsg(db));
        result_msg = strdup("Error: Could not process order due to a database error.");
        goto cleanup;
    }

    rc = sqlite3_bind_int(stmt, 2, quantity);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind quantity: %s\n", sqlite3_errmsg(db));
        result_msg = strdup("Error: Could not process order due to a database error.");
        goto cleanup;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
        result_msg = strdup("Error: Could not process order due to a database error.");
        goto cleanup;
    }

    snprintf(buffer, sizeof(buffer), "Order for %d of %s placed successfully. Redirecting to stock_view...", quantity, stock_name);
    result_msg = strdup(buffer);

cleanup:
    if (stmt) sqlite3_finalize(stmt);
    if (db) sqlite3_close(db);
    return result_msg;
}

void view_stocks() {
    printf("\n--- Current Orders in Database ---\n");
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;

    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        if (db) sqlite3_close(db);
        return;
    }

    const char* sql = "SELECT id, stock_name, quantity FROM orders";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }
    
    bool found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = true;
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* stock = sqlite3_column_text(stmt, 1);
        int quantity = sqlite3_column_int(stmt, 2);
        printf("ID: %d, Stock: %s, Quantity: %d\n", id, stock, quantity);
    }

    if (!found) {
        printf("No orders found.\n");
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    printf("--------------------------------\n\n");
}


void run_test(const char* test_name, const char* stock, int quantity) {
    printf("Test: (%s)\n", test_name);
    printf("Input: stock_name='%s', quantity=%d\n", stock, quantity);
    char* result = buy_order(stock, quantity);
    if (result) {
        printf("Output: %s\n\n", result);
        free(result);
    } else {
        printf("Output: (null) - memory allocation failed\n\n");
    }
}

int main(void) {
    remove(DB_FILE);
    setup_database();

    printf("--- Running Test Cases ---\n");

    // Test Case 1: Valid input
    run_test("Valid", "AAPL", 100);

    // Test Case 2: Invalid quantity
    run_test("Invalid Quantity", "GOOG", -5);

    // Test Case 3: Invalid stock name (too long)
    run_test("Invalid Name - Too Long", "VERYLONGSTOCKNAME", 10);
    
    // Test Case 4: Invalid stock name (invalid characters)
    run_test("Invalid Name - Characters", "AMZN!", 20);

    // Test Case 5: Potentially malicious input (blocked by validation)
    run_test("Malicious Input", "TSLA'; DROP TABLE orders; --", 50);

    view_stocks();
    
    return 0;
}