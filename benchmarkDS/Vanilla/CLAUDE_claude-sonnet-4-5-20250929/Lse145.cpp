
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cstring>

class Lse145 {
private:
    static const char* DB_NAME;
    
public:
    // The buy_function that processes stock name
    static void buy_function(const std::string& stockName) {
        std::cout << "Processing stock: " << stockName << std::endl;
        // Additional stock processing logic can be added here
        std::cout << "Stock " << stockName << " processed successfully" << std::endl;
    }
    
    // The buy_stock function that handles the purchase
    static bool buy_stock(const std::string& stockName, int quantity) {
        sqlite3* db = nullptr;
        char* errMsg = nullptr;
        
        // Open database connection
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        // Create table if not exists
        const char* createTableSQL = 
            "CREATE TABLE IF NOT EXISTS stocks ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "stock_name TEXT, "
            "quantity INTEGER, "
            "purchase_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
        
        rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "Error creating table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        
        // Insert stock data
        sqlite3_stmt* stmt;
        const char* insertSQL = "INSERT INTO stocks (stock_name, quantity) VALUES (?, ?)";
        
        rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, stockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, quantity);
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }
        
        std::cout << "Stock inserted into database: " << stockName 
                  << ", Quantity: " << quantity << std::endl;
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        // Call buy_function with stock name
        buy_function(stockName);
        
        return true;
    }
};

const char* Lse145::DB_NAME = "stocks.db";

int main() {
    std::cout << "=== Test Case 1 ===" << std::endl;
    Lse145::buy_stock("AAPL", 10);
    
    std::cout << "\\n=== Test Case 2 ===" << std::endl;
    Lse145::buy_stock("GOOGL", 5);
    
    std::cout << "\\n=== Test Case 3 ===" << std::endl;
    Lse145::buy_stock("MSFT", 15);
    
    std::cout << "\\n=== Test Case 4 ===" << std::endl;
    Lse145::buy_stock("TSLA", 8);
    
    std::cout << "\\n=== Test Case 5 ===" << std::endl;
    Lse145::buy_stock("AMZN", 3);
    
    return 0;
}
