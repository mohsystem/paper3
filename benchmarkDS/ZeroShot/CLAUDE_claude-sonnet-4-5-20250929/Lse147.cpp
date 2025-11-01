
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>

class Lse147 {
private:
    static constexpr const char* DB_NAME = "stocks.db";
    
public:
    // Initialize database and create table if not exists
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS orders ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "stock_name TEXT NOT NULL, "
                         "quantity INTEGER NOT NULL, "
                         "order_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP);";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    // Secure method to insert buy order using prepared statements
    static bool buyOrder(const std::string& stockName, int quantity) {
        // Input validation
        if (stockName.empty() || stockName.find_first_not_of(" \\t\\n\\r") == std::string::npos) {
            std::cerr << "Error: Stock name cannot be empty" << std::endl;
            return false;
        }
        
        if (quantity <= 0) {
            std::cerr << "Error: Quantity must be positive" << std::endl;
            return false;
        }
        
        // Sanitize stock name
        std::regex validPattern("^[a-zA-Z0-9._-]+$");
        if (!std::regex_match(stockName, validPattern)) {
            std::cerr << "Error: Invalid stock name format" << std::endl;
            return false;
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        const char* sql = "INSERT INTO orders (stock_name, quantity) VALUES (?, ?)";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        // Bind parameters
        sqlite3_bind_text(stmt, 1, stockName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, quantity);
        
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return false;
        }
        
        std::cout << "Order placed successfully!" << std::endl;
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return true;
    }
    
    // Method to view all stock orders
    static void stockView() {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "SELECT id, stock_name, quantity, order_date FROM orders ORDER BY order_date DESC";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return;
        }
        
        std::cout << "\\n=== Stock Orders ===" << std::endl;
        std::cout << "ID    Stock Name      Quantity   Order Date" << std::endl;
        std::cout << std::string(55, '-') << std::endl;
        
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* stockName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            int quantity = sqlite3_column_int(stmt, 2);
            const char* orderDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            
            printf("%-5d %-15s %-10d %-20s\\n", id, stockName, quantity, orderDate);
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
};

int main() {
    Lse147::initializeDatabase();
    
    // Test cases
    std::cout << "Running test cases...\\n" << std::endl;
    
    // Test case 1: Valid order
    std::cout << "Test 1: Valid order" << std::endl;
    Lse147::buyOrder("AAPL", 100);
    
    // Test case 2: Valid order with different stock
    std::cout << "\\nTest 2: Another valid order" << std::endl;
    Lse147::buyOrder("GOOGL", 50);
    
    // Test case 3: Invalid - empty stock name
    std::cout << "\\nTest 3: Empty stock name" << std::endl;
    Lse147::buyOrder("", 25);
    
    // Test case 4: Invalid - negative quantity
    std::cout << "\\nTest 4: Negative quantity" << std::endl;
    Lse147::buyOrder("MSFT", -10);
    
    // Test case 5: Invalid - special characters in stock name
    std::cout << "\\nTest 5: Invalid stock name with SQL injection attempt" << std::endl;
    Lse147::buyOrder("TSLA'; DROP TABLE orders; --", 75);
    
    // Display all orders
    Lse147::stockView();
    
    return 0;
}
