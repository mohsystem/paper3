
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <regex>
#include <cstring>

class Lse145 {
private:
    static const char* DB_NAME;
    
    static std::string sanitizeStockName(const std::string& name) {
        std::regex pattern("[^a-zA-Z0-9]");
        return std::regex_replace(name, pattern, "");
    }
    
public:
    static void initDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Database initialization error: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS stocks ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "stock_name TEXT NOT NULL, "
                         "quantity INTEGER NOT NULL, "
                         "purchase_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    static bool buyFunction(const std::string& stockName) {
        if (stockName.empty()) {
            std::cout << "Invalid stock name" << std::endl;
            return false;
        }
        
        std::cout << "Processing purchase for stock: " << stockName << std::endl;
        return true;
    }
    
    static bool buyStock(std::string stockName, int quantity) {
        // Input validation
        if (stockName.empty()) {
            std::cout << "Error: Stock name cannot be empty" << std::endl;
            return false;
        }
        
        if (quantity <= 0) {
            std::cout << "Error: Quantity must be positive" << std::endl;
            return false;
        }
        
        // Sanitize input
        stockName = sanitizeStockName(stockName);
        
        // Call buy_function
        if (!buyFunction(stockName)) {
            return false;
        }
        
        // Insert into database using parameterized query
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Database error: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        const char* sql = "INSERT INTO stocks(stock_name, quantity) VALUES(?, ?)";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Prepare error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, stockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, quantity);
        
        rc = sqlite3_step(stmt);
        
        if (rc == SQLITE_DONE) {
            std::cout << "Successfully purchased " << quantity << " shares of " << stockName << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return true;
        }
        
        std::cerr << "Execution error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
};

const char* Lse145::DB_NAME = "stocks.db";

int main() {
    Lse145::initDatabase();
    
    std::cout << "=== Test Case 1: Valid purchase ===" << std::endl;
    Lse145::buyStock("AAPL", 10);
    
    std::cout << "\\n=== Test Case 2: Another valid purchase ===" << std::endl;
    Lse145::buyStock("GOOGL", 5);
    
    std::cout << "\\n=== Test Case 3: Invalid quantity (negative) ===" << std::endl;
    Lse145::buyStock("MSFT", -5);
    
    std::cout << "\\n=== Test Case 4: Invalid quantity (zero) ===" << std::endl;
    Lse145::buyStock("TSLA", 0);
    
    std::cout << "\\n=== Test Case 5: Empty stock name ===" << std::endl;
    Lse145::buyStock("", 10);
    
    return 0;
}
