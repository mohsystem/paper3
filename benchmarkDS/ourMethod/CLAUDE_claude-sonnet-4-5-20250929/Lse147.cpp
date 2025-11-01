
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>
#include <cstdlib>
#include <memory>

// Input validation: Whitelist pattern for stock names (alphanumeric and common symbols)
const std::regex STOCK_NAME_PATTERN("^[A-Z0-9._-]{1,10}$");
const int MAX_QUANTITY = 1000000;
const int MIN_QUANTITY = 1;

class DatabaseConnection {
private:
    sqlite3* db;
    
public:
    DatabaseConnection() : db(nullptr) {
        // Secure database connection using environment variables (avoid hardcoded credentials - CWE-798)
        const char* db_path = std::getenv("DB_PATH");
        std::string dbPath = db_path ? db_path : "stocks.db";
        
        // Validate db_path to prevent path traversal (CWE-22)
        if (dbPath.find("..") != std::string::npos || dbPath[0] == '/') {
            std::cerr << "Invalid database path" << std::endl;
            return;
        }
        
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database" << std::endl;
            db = nullptr;
        }
    }
    
    ~DatabaseConnection() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }
    
    sqlite3* get() { return db; }
    bool isValid() { return db != nullptr; }
};

// Input validation function to prevent injection attacks (CWE-89)
bool isValidStockName(const std::string& stockName) {
    if (stockName.empty()) {
        return false;
    }
    return std::regex_match(stockName, STOCK_NAME_PATTERN);
}

bool isValidQuantity(int quantity) {
    return quantity >= MIN_QUANTITY && quantity <= MAX_QUANTITY;
}

// Secure buy order function using parameterized queries (prevents SQL injection)
bool buyOrder(const std::string& stockName, int quantity) {
    // Input validation: Check all inputs before processing (Rules#3)
    if (!isValidStockName(stockName)) {
        std::cerr << "Invalid stock name format" << std::endl;
        return false;
    }
    
    if (!isValidQuantity(quantity)) {
        std::cerr << "Invalid quantity range" << std::endl;
        return false;
    }
    
    DatabaseConnection dbConn;
    if (!dbConn.isValid()) {
        std::cerr << "Database connection failed" << std::endl;
        return false;
    }
    
    sqlite3_stmt* stmt = nullptr;
    
    // Use parameterized query to prevent SQL injection (CWE-89)
    const char* sql = "INSERT INTO orders (stock_name, quantity, order_date) VALUES (?, ?, datetime('now'))";
    
    int rc = sqlite3_prepare_v2(dbConn.get(), sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement" << std::endl;
        return false;
    }
    
    // RAII wrapper for automatic cleanup
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmtGuard(stmt, sqlite3_finalize);
    
    // Bind parameters - SQLite handles escaping automatically
    sqlite3_bind_text(stmt, 1, stockName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, quantity);
    
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_DONE) {
        // Log success without exposing sensitive data (Rules#1)
        std::cout << "Order placed successfully" << std::endl;
        return true;
    } else {
        // Error handling: Don't expose internal details to user (Rules#1)\n        std::cerr << "Order insertion failed" << std::endl;\n        return false;\n    }\n}\n\n// Initialize database table\nvoid initializeDatabase() {\n    DatabaseConnection dbConn;\n    if (!dbConn.isValid()) {\n        std::cerr << "Database initialization failed" << std::endl;\n        return;\n    }\n    \n    const char* createTable = \n        "CREATE TABLE IF NOT EXISTS orders ("\n        "id INTEGER PRIMARY KEY AUTOINCREMENT, "\n        "stock_name TEXT NOT NULL, "\n        "quantity INTEGER NOT NULL, "\n        "order_date TEXT NOT NULL)";\n    \n    char* errMsg = nullptr;\n    int rc = sqlite3_exec(dbConn.get(), createTable, nullptr, nullptr, &errMsg);\n    \n    if (rc != SQLITE_OK) {\n        std::cerr << "Table creation failed" << std::endl;\n        if (errMsg) {\n            sqlite3_free(errMsg);\n        }\n    }\n}\n\nint main() {\n    // Initialize database\n    initializeDatabase();\n    \n    // Test cases with validation\n    std::cout << "Test Case 1: Valid order" << std::endl;\n    std::cout << "Result: " << (buyOrder("AAPL", 100) ? "true" : "false") << std::endl;\n    \n    std::cout << "\\nTest Case 2: Valid order with different stock" << std::endl;\n    std::cout << "Result: " << (buyOrder("GOOGL", 50) ? "true" : "false") << std::endl;\n    \n    std::cout << "\\nTest Case 3: Invalid stock name (SQL injection attempt)" << std::endl;\n    std::cout << "Result: " << (buyOrder("'; DROP TABLE orders; --", 10) ? "true" : "false") << std::endl;\n    \n    std::cout << "\
Test Case 4: Invalid quantity (negative)" << std::endl;\n    std::cout << "Result: " << (buyOrder("MSFT", -5) ? "true" : "false") << std::endl;\n    \n    std::cout << "\
Test Case 5: Invalid quantity (exceeds maximum)" << std::endl;\n    std::cout << "Result: " << (buyOrder("TSLA", 2000000) ? "true" : "false") << std::endl;
    
    return 0;
}
