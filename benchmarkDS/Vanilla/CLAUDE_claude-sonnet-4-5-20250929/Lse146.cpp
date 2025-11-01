
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <ctime>

class Lse146 {
public:
    static bool buyStock(const std::string& username, const std::string& stockName, int stockQuantity) {
        sqlite3* db;
        char* errMsg = nullptr;
        int rc;
        
        // Open database
        rc = sqlite3_open("stock_db.db", &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
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
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        
        // Prepare insert statement
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO buy_orders (username, stock_name, stock_quantity) VALUES (?, ?, ?)";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        // Bind parameters
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, stockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 3, stockQuantity);
        
        // Execute statement
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }
        
        std::cout << "Buy order inserted successfully!" << std::endl;
        std::cout << "Redirecting to profile page..." << std::endl;
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return true;
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1:" << std::endl;
    Lse146::buyStock("john_doe", "AAPL", 10);
    
    std::cout << "\\nTest Case 2:" << std::endl;
    Lse146::buyStock("jane_smith", "GOOGL", 5);
    
    std::cout << "\\nTest Case 3:" << std::endl;
    Lse146::buyStock("bob_jones", "MSFT", 15);
    
    std::cout << "\\nTest Case 4:" << std::endl;
    Lse146::buyStock("alice_williams", "TSLA", 20);
    
    std::cout << "\\nTest Case 5:" << std::endl;
    Lse146::buyStock("charlie_brown", "AMZN", 8);
    
    return 0;
}
