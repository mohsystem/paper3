
#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include <cstring>
#include <sqlite3.h>

// Secure constants for validation
const int MAX_QUANTITY = 1000000;
const int MIN_QUANTITY = 1;
const int MAX_USERNAME_LEN = 50;
const int MIN_USERNAME_LEN = 3;
const int MAX_STOCK_NAME_LEN = 10;

class SecureOrderProcessor {
private:
    std::regex stockNamePattern;
    std::regex usernamePattern;
    
public:
    SecureOrderProcessor() : 
        stockNamePattern("^[A-Za-z0-9._-]{1,10}$"),
        usernamePattern("^[A-Za-z0-9_]{3,50}$") {}
    
    bool validateUsername(const std::string& username) {
        // Check for null or empty
        if (username.empty()) {
            std::cerr << "Invalid username: empty" << std::endl;
            return false;
        }
        
        // Check length bounds
        if (username.length() < MIN_USERNAME_LEN || username.length() > MAX_USERNAME_LEN) {
            std::cerr << "Username length must be between " << MIN_USERNAME_LEN 
                      << " and " << MAX_USERNAME_LEN << std::endl;
            return false;
        }
        
        // Check format using regex
        if (!std::regex_match(username, usernamePattern)) {
            std::cerr << "Username contains invalid characters" << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool validateStockName(const std::string& stockName) {
        // Check for null or empty
        if (stockName.empty()) {
            std::cerr << "Invalid stock name: empty" << std::endl;
            return false;
        }
        
        // Check length
        if (stockName.length() > MAX_STOCK_NAME_LEN) {
            std::cerr << "Stock name too long" << std::endl;
            return false;
        }
        
        // Check format to prevent injection
        if (!std::regex_match(stockName, stockNamePattern)) {
            std::cerr << "Stock name contains invalid characters" << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool validateQuantity(int quantity) {
        // Check bounds to prevent overflow and negative values
        if (quantity < MIN_QUANTITY || quantity > MAX_QUANTITY) {
            std::cerr << "Quantity must be between " << MIN_QUANTITY 
                      << " and " << MAX_QUANTITY << std::endl;
            return false;
        }
        
        return true;
    }
    
    bool insertBuyOrder(const std::string& username, const std::string& stockName, 
                       int stockQuantity) {
        // Validate all inputs before processing
        if (!validateUsername(username)) {
            return false;
        }
        
        if (!validateStockName(stockName)) {
            return false;
        }
        
        if (!validateQuantity(stockQuantity)) {
            return false;
        }
        
        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;
        int rc;
        
        // Get database path from environment (secure configuration)
        const char* dbPath = std::getenv("DB_PATH");
        if (dbPath == nullptr) {
            dbPath = "orders.db";
        }
        
        // Validate database path to prevent directory traversal
        std::string dbPathStr(dbPath);
        if (dbPathStr.find("..") != std::string::npos || dbPathStr[0] == '/') {
            std::cerr << "Invalid database path" << std::endl;
            return false;
        }
        
        // Open database connection
        rc = sqlite3_open(dbPath, &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Database connection error" << std::endl;
            if (db != nullptr) {
                sqlite3_close(db);
            }
            return false;
        }
        
        // Prepare parameterized query to prevent SQL injection
        const char* sql = "INSERT INTO buy_orders (username, stock_name, quantity, order_date) "
                         "VALUES (?, ?, ?, datetime('now'))";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Statement preparation error" << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        // Bind validated parameters
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, stockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, stockQuantity);
        
        // Execute the statement
        rc = sqlite3_step(stmt);
        
        bool success = (rc == SQLITE_DONE);
        
        if (success) {
            std::cout << "Order inserted successfully" << std::endl;
        } else {
            std::cerr << "Failed to insert order" << std::endl;
        }
        
        // Clean up resources
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return success;
    }
};

int main() {
    SecureOrderProcessor processor;
    
    // Test case 1: Valid input
    std::cout << "Test 1: " << processor.insertBuyOrder("john_doe", "AAPL", 100) << std::endl;
    
    // Test case 2: Valid input with different stock
    std::cout << "Test 2: " << processor.insertBuyOrder("jane_smith", "GOOGL", 50) << std::endl;
    
    // Test case 3: Invalid stock name (SQL injection attempt)
    std::cout << "Test 3: " << processor.insertBuyOrder("user123", "AAPL'; DROP TABLE--", 10) << std::endl;
    
    // Test case 4: Invalid quantity (negative)
    std::cout << "Test 4: " << processor.insertBuyOrder("alice_w", "MSFT", -5) << std::endl;
    
    // Test case 5: Invalid quantity (exceeds maximum)
    std::cout << "Test 5: " << processor.insertBuyOrder("bob_jones", "TSLA", 2000000) << std::endl;
    
    return 0;
}
