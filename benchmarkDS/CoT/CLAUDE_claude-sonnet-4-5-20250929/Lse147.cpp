
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

const std::string DB_NAME = "stocks.db";
const std::regex STOCK_NAME_PATTERN("^[a-zA-Z0-9_-]{1,50}$");

void initializeDatabase() {
    sqlite3* db;
    char* errMsg = nullptr;
    
    if (sqlite3_open(DB_NAME.c_str(), &db) == SQLITE_OK) {
        const char* sql = "CREATE TABLE IF NOT EXISTS orders ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "stock_name TEXT NOT NULL,"
                         "quantity INTEGER NOT NULL,"
                         "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
        
        sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (errMsg) {
            sqlite3_free(errMsg);
        }
    }
    sqlite3_close(db);
}

bool validateStockName(const std::string& stockName) {
    return !stockName.empty() && std::regex_match(stockName, STOCK_NAME_PATTERN);
}

bool validateQuantity(int quantity) {
    return quantity > 0;
}

bool buyOrder(const std::string& stockName, int quantity) {
    // Input validation
    if (!validateStockName(stockName)) {
        std::cout << "Error: Invalid stock name format" << std::endl;
        return false;
    }
    
    if (!validateQuantity(quantity)) {
        std::cout << "Error: Quantity must be positive" << std::endl;
        return false;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    if (sqlite3_open(DB_NAME.c_str(), &db) != SQLITE_OK) {
        std::cout << "Error opening database" << std::endl;
        return false;
    }
    
    // Parameterized query prevents SQL injection
    const char* sql = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, stockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, quantity);
        
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            std::cout << "Order placed successfully: " << stockName 
                     << " x " << quantity << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return true;
        }
    }
    
    std::cout << "Error placing order" << std::endl;
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
}

void stockView() {
    std::cout << "\\n=== Stock View ===" << std::endl;
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    if (sqlite3_open(DB_NAME.c_str(), &db) != SQLITE_OK) {
        std::cout << "Error opening database" << std::endl;
        return;
    }
    
    const char* sql = "SELECT id, stock_name, quantity, order_date FROM orders";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::cout << "ID: " << sqlite3_column_int(stmt, 0)
                     << ", Stock: " << sqlite3_column_text(stmt, 1)
                     << ", Quantity: " << sqlite3_column_int(stmt, 2)
                     << ", Date: " << sqlite3_column_text(stmt, 3) << std::endl;
        }
        sqlite3_finalize(stmt);
    }
    
    sqlite3_close(db);
}

int main() {
    // Initialize database
    initializeDatabase();
    
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    buyOrder("AAPL", 10);
    
    std::cout << "\\nTest Case 2:" << std::endl;
    buyOrder("GOOGL", 5);
    
    std::cout << "\\nTest Case 3:" << std::endl;
    buyOrder("MSFT", 15);
    
    std::cout << "\\nTest Case 4 (Invalid - negative quantity):" << std::endl;
    buyOrder("TSLA", -5);
    
    std::cout << "\\nTest Case 5 (Invalid - SQL injection attempt):" << std::endl;
    buyOrder("AAPL'; DROP TABLE orders;--", 10);
    
    // Display all orders
    stockView();
    
    return 0;
}
