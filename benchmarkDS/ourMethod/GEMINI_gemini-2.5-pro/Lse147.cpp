#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <cstdio> // For remove()
#include "sqlite3.h"

// Compile with: g++ main.cpp -lsqlite3 -o main

const char* DB_FILE = "orders_cpp.db";
const int MAX_STOCK_NAME_LENGTH = 10;
// Simple regex for stock symbols: 1-10 uppercase letters and dots.
const std::regex STOCK_NAME_PATTERN("^[A-Z.]{1," + std::to_string(MAX_STOCK_NAME_LENGTH) + "}$");

void setup_database() {
    sqlite3* db = nullptr;
    char* zErrMsg = nullptr;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }

    const char* sql = "CREATE TABLE IF NOT EXISTS orders ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "stock_name TEXT NOT NULL,"
                      "quantity INTEGER NOT NULL);";

    rc = sqlite3_exec(db, sql, nullptr, nullptr, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    sqlite3_close(db);
}

std::string buy_order(const std::string& stock_name, int quantity) {
    // Rule #1: Ensure all input is validated
    if (!std::regex_match(stock_name, STOCK_NAME_PATTERN)) {
        return "Error: Invalid stock name format. Must be 1-" + std::to_string(MAX_STOCK_NAME_LENGTH) + " uppercase letters/dots.";
    }
    if (quantity <= 0) {
        return "Error: Quantity must be a positive integer.";
    }

    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        if(db) sqlite3_close(db);
        return "Error: Could not process order due to a database error.";
    }

    const char* sql = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?);";
    
    // Rule #2 & #5: Use prepared statements to prevent SQL injection
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return "Error: Could not process order due to a database error.";
    }

    sqlite3_bind_text(stmt, 1, stock_name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, quantity);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return "Error: Could not process order due to a database error.";
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);

    return "Order for " + std::to_string(quantity) + " of " + stock_name + " placed successfully. Redirecting to stock_view...";
}

void view_stocks() {
    std::cout << "\n--- Current Orders in Database ---" << std::endl;
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        if(db) sqlite3_close(db);
        return;
    }

    const char* sql = "SELECT id, stock_name, quantity FROM orders";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return;
    }
    
    bool found = false;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        found = true;
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* stock = sqlite3_column_text(stmt, 1);
        int quantity = sqlite3_column_int(stmt, 2);
        std::cout << "ID: " << id << ", Stock: " << stock << ", Quantity: " << quantity << std::endl;
    }
    
    if (!found) {
        std::cout << "No orders found." << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    std::cout << "--------------------------------\n" << std::endl;
}

int main() {
    remove(DB_FILE);
    setup_database();

    std::cout << "--- Running Test Cases ---" << std::endl;

    // Test Case 1: Valid input
    std::cout << "Test 1: (Valid)" << std::endl;
    std::cout << "Input: stock_name='AAPL', quantity=100" << std::endl;
    std::cout << "Output: " << buy_order("AAPL", 100) << std::endl << std::endl;

    // Test Case 2: Invalid quantity
    std::cout << "Test 2: (Invalid Quantity)" << std::endl;
    std::cout << "Input: stock_name='GOOG', quantity=-5" << std::endl;
    std::cout << "Output: " << buy_order("GOOG", -5) << std::endl << std::endl;

    // Test Case 3: Invalid stock name (too long)
    std::cout << "Test 3: (Invalid Name - Too Long)" << std::endl;
    std::cout << "Input: stock_name='VERYLONGSTOCKNAME', quantity=10" << std::endl;
    std::cout << "Output: " << buy_order("VERYLONGSTOCKNAME", 10) << std::endl << std::endl;
    
    // Test Case 4: Invalid stock name (invalid characters)
    std::cout << "Test 4: (Invalid Name - Characters)" << std::endl;
    std::cout << "Input: stock_name='AMZN!', quantity=20" << std::endl;
    std::cout << "Output: " << buy_order("AMZN!", 20) << std::endl << std::endl;
    
    // Test Case 5: Potentially malicious input (blocked by validation)
    std::cout << "Test 5: (Malicious Input)" << std::endl;
    std::cout << "Input: stock_name='TSLA'; DROP TABLE orders; --', quantity=50" << std::endl;
    std::cout << "Output: " << buy_order("TSLA'; DROP TABLE orders; --", 50) << std::endl << std::endl;

    view_stocks();
    
    return 0;
}