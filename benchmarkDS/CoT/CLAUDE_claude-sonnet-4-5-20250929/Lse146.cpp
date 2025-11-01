
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

class Lse146 {
private:
    static const std::regex USERNAME_PATTERN;
    static const std::regex STOCK_NAME_PATTERN;
    
public:
    static bool validateUsername(const std::string& username) {
        return !username.empty() && std::regex_match(username, USERNAME_PATTERN);
    }
    
    static bool validateStockName(const std::string& stockName) {
        return !stockName.empty() && std::regex_match(stockName, STOCK_NAME_PATTERN);
    }
    
    static bool validateQuantity(int quantity) {
        return quantity > 0 && quantity <= 1000000;
    }
    
    static std::string insertBuyOrder(const std::string& username, 
                                      const std::string& stockName, 
                                      int stockQuantity) {
        if (!validateUsername(username)) {
            return "Error: Invalid username format";
        }
        if (!validateStockName(stockName)) {
            return "Error: Invalid stock name format";
        }
        if (!validateQuantity(stockQuantity)) {
            return "Error: Invalid quantity (must be 1-1000000)";
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        int rc = sqlite3_open("stock_orders.db", &db);
        
        if (rc != SQLITE_OK) {
            std::string error = "Error: Cannot open database - " + 
                              std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        const char* insertSQL = 
            "INSERT INTO buy_orders (username, stock_name, quantity, order_date) "
            "VALUES (?, ?, ?, datetime('now'))";
        
        rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            std::string error = "Error: Failed to prepare statement - " + 
                              std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, stockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, stockQuantity);
        
        rc = sqlite3_step(stmt);
        
        std::string result;
        if (rc == SQLITE_DONE) {
            result = "redirect:/profile?user=" + username;
        } else {
            result = "Error: Failed to insert order - " + 
                    std::string(sqlite3_errmsg(db));
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
};

const std::regex Lse146::USERNAME_PATTERN("^[a-zA-Z0-9_]{3,30}$");
const std::regex Lse146::STOCK_NAME_PATTERN("^[a-zA-Z0-9_\\\\-\\\\.]{1,20}$");

int main() {
    std::cout << "=== Stock Order System Test Cases ===\\n\\n";
    
    // Test Case 1: Valid order
    std::cout << "Test 1 - Valid order:\\n";
    std::cout << Lse146::insertBuyOrder("john_doe", "AAPL", 100) << "\\n\\n";
    
    // Test Case 2: Another valid order
    std::cout << "Test 2 - Valid order with different stock:\\n";
    std::cout << Lse146::insertBuyOrder("jane_smith", "GOOGL", 50) << "\\n\\n";
    
    // Test Case 3: Invalid username
    std::cout << "Test 3 - Invalid username:\\n";
    std::cout << Lse146::insertBuyOrder("user@123", "MSFT", 75) << "\\n\\n";
    
    // Test Case 4: Invalid stock name
    std::cout << "Test 4 - Invalid stock name:\\n";
    std::cout << Lse146::insertBuyOrder("testuser", "STOCK'; DROP TABLE--", 25) << "\\n\\n";
    
    // Test Case 5: Invalid quantity
    std::cout << "Test 5 - Invalid quantity:\\n";
    std::cout << Lse146::insertBuyOrder("alice", "TSLA", -10) << "\\n";
    
    return 0;
}
