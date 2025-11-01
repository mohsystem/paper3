
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

class Lse146 {
public:
    static bool processBuyOrder(const std::string& username, const std::string& stockName, int stockQuantity) {
        // Input validation
        if (username.empty() || username.find_first_not_of(' ') == std::string::npos) {
            std::cout << "Error: Username cannot be empty" << std::endl;
            return false;
        }
        
        if (stockName.empty() || stockName.find_first_not_of(' ') == std::string::npos) {
            std::cout << "Error: Stock name cannot be empty" << std::endl;
            return false;
        }
        
        if (stockQuantity <= 0) {
            std::cout << "Error: Stock quantity must be positive" << std::endl;
            return false;
        }
        
        // Sanitize inputs
        std::string cleanUsername = trim(username);
        std::string cleanStockName = sanitizeStockName(stockName);
        
        sqlite3* db;
        char* errMsg = nullptr;
        
        // Open database
        int rc = sqlite3_open("trading.db", &db);
        if (rc) {
            std::cout << "Database error: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        // Create table if not exists
        const char* createTableSQL = 
            "CREATE TABLE IF NOT EXISTS buy_orders ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT NOT NULL,"
            "stock_name TEXT NOT NULL,"
            "stock_quantity INTEGER NOT NULL,"
            "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
        
        rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cout << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        
        // Use parameterized query to prevent SQL injection
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO buy_orders (username, stock_name, stock_quantity) VALUES (?, ?, ?)";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cout << "SQL error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, cleanUsername.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, cleanStockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, stockQuantity);
        
        rc = sqlite3_step(stmt);
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (rc == SQLITE_DONE) {
            std::cout << "Buy order placed successfully!" << std::endl;
            std::cout << "Redirecting to profile page..." << std::endl;
            return true;
        }
        
        return false;
    }

private:
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(' ');
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(' ');
        return str.substr(first, (last - first + 1));
    }
    
    static std::string sanitizeStockName(const std::string& str) {
        std::string result = trim(str);
        std::regex pattern("[^a-zA-Z0-9]");
        return std::regex_replace(result, pattern, "");
    }
};

int main() {
    std::cout << "Testing Buy Order Processing System\\n" << std::endl;
    
    // Test case 1: Valid order
    std::cout << "Test 1: Valid buy order" << std::endl;
    Lse146::processBuyOrder("john_doe", "AAPL", 100);
    std::cout << std::endl;
    
    // Test case 2: Another valid order
    std::cout << "Test 2: Valid buy order for different stock" << std::endl;
    Lse146::processBuyOrder("jane_smith", "GOOGL", 50);
    std::cout << std::endl;
    
    // Test case 3: Invalid - empty username
    std::cout << "Test 3: Empty username (should fail)" << std::endl;
    Lse146::processBuyOrder("", "TSLA", 75);
    std::cout << std::endl;
    
    // Test case 4: Invalid - negative quantity
    std::cout << "Test 4: Negative quantity (should fail)" << std::endl;
    Lse146::processBuyOrder("bob_jones", "MSFT", -10);
    std::cout << std::endl;
    
    // Test case 5: Invalid - empty stock name
    std::cout << "Test 5: Empty stock name (should fail)" << std::endl;
    Lse146::processBuyOrder("alice_wilson", "", 200);
    
    return 0;
}
