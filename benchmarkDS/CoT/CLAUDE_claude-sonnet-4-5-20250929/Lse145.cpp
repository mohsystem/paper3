
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>
#include <algorithm>
#include <cctype>

class Lse145 {
private:
    static const std::string DB_NAME;
    
    static std::string toUpper(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), ::toupper);
        return result;
    }
    
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \\t\\n\\r");
        if (first == std::string::npos) return "";
        size_t last = str.find_last_not_of(" \\t\\n\\r");
        return str.substr(first, (last - first + 1));
    }
    
public:
    static void buyFunction(const std::string& stockName) {
        if (stockName.empty()) {
            std::cout << "Error: Invalid stock name in buy_function" << std::endl;
            return;
        }
        std::cout << "Processing purchase for stock: " << stockName << std::endl;
    }
    
    static bool buyStock(const std::string& stockName, int quantity) {
        // Input validation
        if (stockName.empty() || trim(stockName).empty()) {
            std::cout << "Error: Stock name cannot be empty" << std::endl;
            return false;
        }
        
        std::string cleanStockName = toUpper(trim(stockName));
        
        std::regex stockPattern("^[A-Z]{1,5}$");
        if (!std::regex_match(cleanStockName, stockPattern)) {
            std::cout << "Error: Invalid stock name format" << std::endl;
            return false;
        }
        
        if (quantity <= 0 || quantity > 10000) {
            std::cout << "Error: Quantity must be between 1 and 10000" << std::endl;
            return false;
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cout << "Database error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        const char* sql = "INSERT INTO stocks (stock_name, quantity) VALUES (?, ?)";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cout << "Database error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, cleanStockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, quantity);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (rc != SQLITE_DONE) {
            std::cout << "Database error during insert" << std::endl;
            return false;
        }
        
        buyFunction(cleanStockName);
        std::cout << "Stock purchased successfully" << std::endl;
        return true;
    }
    
    static void initDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cout << "Error opening database" << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS stocks ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "stock_name TEXT NOT NULL, "
                         "quantity INTEGER NOT NULL)";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cout << "Error creating table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
};

const std::string Lse145::DB_NAME = "stocks.db";

int main() {
    Lse145::initDatabase();
    
    std::cout << "Test 1: Valid purchase" << std::endl;
    Lse145::buyStock("AAPL", 100);
    
    std::cout << "\\nTest 2: Valid purchase" << std::endl;
    Lse145::buyStock("GOOGL", 50);
    
    std::cout << "\\nTest 3: Invalid stock name (too long)" << std::endl;
    Lse145::buyStock("TOOLONG", 10);
    
    std::cout << "\\nTest 4: Invalid quantity (negative)" << std::endl;
    Lse145::buyStock("MSFT", -5);
    
    std::cout << "\\nTest 5: Invalid stock name (empty)" << std::endl;
    Lse145::buyStock("", 20);
    
    return 0;
}
