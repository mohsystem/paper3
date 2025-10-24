
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>
#include <memory>
#include <cstring>

class Task137 {
private:
    const std::string DB_PATH = "products.db";
    std::regex productIdPattern;
    
public:
    Task137() : productIdPattern("^[a-zA-Z0-9_-]{1,50}$") {
        initializeDatabase();
    }
    
    void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* dropTable = "DROP TABLE IF EXISTS products";
        sqlite3_exec(db, dropTable, nullptr, nullptr, &errMsg);
        
        const char* createTable = 
            "CREATE TABLE products ("
            "id TEXT PRIMARY KEY, "
            "name TEXT NOT NULL, "
            "description TEXT, "
            "price REAL NOT NULL, "
            "stock INTEGER NOT NULL)";
        
        if (sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "Create table error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            return;
        }
        
        const char* insertSQL = "INSERT INTO products (id, name, description, price, stock) VALUES (?, ?, ?, ?, ?)";
        sqlite3_stmt* stmt;
        
        std::string products[][5] = {
            {"PROD001", "Laptop", "High-performance laptop", "999.99", "50"},
            {"PROD002", "Mouse", "Wireless mouse", "29.99", "200"},
            {"PROD003", "Keyboard", "Mechanical keyboard", "79.99", "150"},
            {"PROD004", "Monitor", "27-inch 4K monitor", "399.99", "75"},
            {"PROD005", "Headphones", "Noise-cancelling headphones", "199.99", "100"}
        };
        
        for (const auto& product : products) {
            if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, product[0].c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, product[1].c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, product[2].c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_double(stmt, 4, std::stod(product[3]));
                sqlite3_bind_int(stmt, 5, std::stoi(product[4]));
                
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
        
        sqlite3_close(db);
    }
    
    bool validateInput(const std::string& input) {
        if (input.empty()) {
            return false;
        }
        return std::regex_match(input, productIdPattern);
    }
    
    void getProductDetails(const std::string& productId) {
        if (!validateInput(productId)) {
            std::cout << "Invalid product ID format. Please use alphanumeric characters, hyphens, or underscores only." << std::endl;
            return;
        }
        
        sqlite3* db;
        if (sqlite3_open(DB_PATH.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* query = "SELECT id, name, description, price, stock FROM products WHERE id = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, productId.c_str(), -1, SQLITE_TRANSIENT);
            
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                std::cout << "Product Found:" << std::endl;
                std::cout << "ID: " << sqlite3_column_text(stmt, 0) << std::endl;
                std::cout << "Name: " << sqlite3_column_text(stmt, 1) << std::endl;
                std::cout << "Description: " << sqlite3_column_text(stmt, 2) << std::endl;
                std::cout << "Price: $" << sqlite3_column_double(stmt, 3) << std::endl;
                std::cout << "Stock: " << sqlite3_column_int(stmt, 4) << std::endl;
            } else {
                std::cout << "Product not found with ID: " << productId << std::endl;
            }
            
            sqlite3_finalize(stmt);
        } else {
            std::cerr << "Query preparation error: " << sqlite3_errmsg(db) << std::endl;
        }
        
        sqlite3_close(db);
    }
};

int main() {
    Task137 task;
    
    std::cout << "Test Case 1: Valid product ID" << std::endl;
    task.getProductDetails("PROD001");
    
    std::cout << "\\nTest Case 2: Valid product ID" << std::endl;
    task.getProductDetails("PROD002");
    
    std::cout << "\\nTest Case 3: Non-existent product" << std::endl;
    task.getProductDetails("PROD999");
    
    std::cout << "\\nTest Case 4: Invalid input (SQL injection attempt)" << std::endl;
    task.getProductDetails("PROD001' OR '1'='1");
    
    std::cout << "\\nTest Case 5: Empty input" << std::endl;
    task.getProductDetails("");
    
    return 0;
}
