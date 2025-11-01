
#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>

class Lse147 {
private:
    static const char* DB_NAME;
    
public:
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* createTableSQL = 
            "CREATE TABLE IF NOT EXISTS orders ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "stock_name TEXT NOT NULL, "
            "quantity INTEGER NOT NULL, "
            "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
        
        rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "Error creating table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    static bool buyOrder(const std::string& stockName, int quantity) {
        if (stockName.empty() || quantity <= 0) {
            return false;
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        const char* insertSQL = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?)";
        
        rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, stockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, quantity);
        
        rc = sqlite3_step(stmt);
        bool success = (rc == SQLITE_DONE);
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return success;
    }
    
    static std::vector<std::string> stockView() {
        std::vector<std::string> orders;
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Error opening database: " << sqlite3_errmsg(db) << std::endl;
            return orders;
        }
        
        const char* selectSQL = "SELECT * FROM orders ORDER BY order_date DESC";
        
        rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return orders;
        }
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* stockName = sqlite3_column_text(stmt, 1);
            int quantity = sqlite3_column_int(stmt, 2);
            const unsigned char* orderDate = sqlite3_column_text(stmt, 3);
            
            std::string order = "ID: " + std::to_string(id) + 
                              ", Stock: " + std::string((char*)stockName) +
                              ", Quantity: " + std::to_string(quantity) +
                              ", Date: " + std::string((char*)orderDate);
            orders.push_back(order);
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return orders;
    }
};

const char* Lse147::DB_NAME = "stocks.db";

int main() {
    Lse147::initializeDatabase();
    
    std::cout << "Test 1: Adding AAPL with quantity 10" << std::endl;
    bool result1 = Lse147::buyOrder("AAPL", 10);
    std::cout << "Result: " << (result1 ? "Success" : "Failed") << std::endl;
    
    std::cout << "\\nTest 2: Adding GOOGL with quantity 5" << std::endl;
    bool result2 = Lse147::buyOrder("GOOGL", 5);
    std::cout << "Result: " << (result2 ? "Success" : "Failed") << std::endl;
    
    std::cout << "\\nTest 3: Adding empty stock name with quantity 15" << std::endl;
    bool result3 = Lse147::buyOrder("", 15);
    std::cout << "Result: " << (result3 ? "Success" : "Failed") << std::endl;
    
    std::cout << "\\nTest 4: Adding MSFT with quantity -5" << std::endl;
    bool result4 = Lse147::buyOrder("MSFT", -5);
    std::cout << "Result: " << (result4 ? "Success" : "Failed") << std::endl;
    
    std::cout << "\\nTest 5: Adding TSLA with quantity 20" << std::endl;
    bool result5 = Lse147::buyOrder("TSLA", 20);
    std::cout << "Result: " << (result5 ? "Success" : "Failed") << std::endl;
    
    std::cout << "\\n=== Stock View ===" << std::endl;
    std::vector<std::string> orders = Lse147::stockView();
    for (const auto& order : orders) {
        std::cout << order << std::endl;
    }
    
    return 0;
}
