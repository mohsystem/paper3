
#include <iostream>
#include <string>
#include <vector>
#include <sqlite3.h>
#include <memory>

class Task137 {
private:
    sqlite3* db;
    
    struct Product {
        int id;
        std::string name;
        double price;
        std::string category;
        
        void print() const {
            std::cout << "Product{id=" << id 
                     << ", name='" << name 
                     << "', price=" << price 
                     << ", category='" << category << "'}" << std::endl;
        }
    };

public:
    Task137() : db(nullptr) {}
    
    ~Task137() {
        if (db) {
            sqlite3_close(db);
        }
    }
    
    bool initializeDatabase() {
        int rc = sqlite3_open(":memory:", &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        const char* create_table = 
            "CREATE TABLE IF NOT EXISTS products ("
            "id INTEGER PRIMARY KEY, "
            "name TEXT NOT NULL, "
            "price REAL NOT NULL, "
            "category TEXT NOT NULL)";
        
        char* errMsg;
        rc = sqlite3_exec(db, create_table, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }
        
        const char* inserts[] = {
            "INSERT INTO products VALUES (1, 'Laptop', 999.99, 'Electronics')",
            "INSERT INTO products VALUES (2, 'Mouse', 29.99, 'Electronics')",
            "INSERT INTO products VALUES (3, 'Desk Chair', 199.99, 'Furniture')",
            "INSERT INTO products VALUES (4, 'Notebook', 4.99, 'Stationery')",
            "INSERT INTO products VALUES (5, 'Monitor', 299.99, 'Electronics')"
        };
        
        for (const char* insert : inserts) {
            rc = sqlite3_exec(db, insert, nullptr, nullptr, &errMsg);
            if (rc != SQLITE_OK) {
                std::cerr << "SQL error: " << errMsg << std::endl;
                sqlite3_free(errMsg);
                return false;
            }
        }
        
        return true;
    }
    
    std::vector<Product> queryProductById(int productId) {
        std::vector<Product> results;
        const char* query = "SELECT * FROM products WHERE id = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement" << std::endl;
            return results;
        }
        
        sqlite3_bind_int(stmt, 1, productId);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Product p;
            p.id = sqlite3_column_int(stmt, 0);
            p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            p.price = sqlite3_column_double(stmt, 2);
            p.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            results.push_back(p);
        }
        
        sqlite3_finalize(stmt);
        return results;
    }
    
    std::vector<Product> queryProductByName(const std::string& productName) {
        std::vector<Product> results;
        const char* query = "SELECT * FROM products WHERE name LIKE ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement" << std::endl;
            return results;
        }
        
        std::string searchPattern = "%" + productName + "%";
        sqlite3_bind_text(stmt, 1, searchPattern.c_str(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Product p;
            p.id = sqlite3_column_int(stmt, 0);
            p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            p.price = sqlite3_column_double(stmt, 2);
            p.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            results.push_back(p);
        }
        
        sqlite3_finalize(stmt);
        return results;
    }
    
    std::vector<Product> queryProductByCategory(const std::string& category) {
        std::vector<Product> results;
        const char* query = "SELECT * FROM products WHERE category = ?";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement" << std::endl;
            return results;
        }
        
        sqlite3_bind_text(stmt, 1, category.c_str(), -1, SQLITE_TRANSIENT);
        
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Product p;
            p.id = sqlite3_column_int(stmt, 0);
            p.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            p.price = sqlite3_column_double(stmt, 2);
            p.category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            results.push_back(p);
        }
        
        sqlite3_finalize(stmt);
        return results;
    }
};

int main() {
    Task137 task;
    
    if (!task.initializeDatabase()) {
        std::cerr << "Failed to initialize database" << std::endl;
        return 1;
    }
    
    std::cout << "Test Case 1: Query product by ID = 1" << std::endl;
    auto test1 = task.queryProductById(1);
    for (const auto& p : test1) p.print();
    
    std::cout << "\\nTest Case 2: Query product by name 'Mouse'" << std::endl;
    auto test2 = task.queryProductByName("Mouse");
    for (const auto& p : test2) p.print();
    
    std::cout << "\\nTest Case 3: Query products by category 'Electronics'" << std::endl;
    auto test3 = task.queryProductByCategory("Electronics");
    for (const auto& p : test3) p.print();
    
    std::cout << "\\nTest Case 4: Query product by partial name 'ote'" << std::endl;
    auto test4 = task.queryProductByName("ote");
    for (const auto& p : test4) p.print();
    
    std::cout << "\\nTest Case 5: Query non-existent product ID = 999" << std::endl;
    auto test5 = task.queryProductById(999);
    if (test5.empty()) {
        std::cout << "No products found" << std::endl;
    } else {
        for (const auto& p : test5) p.print();
    }
    
    return 0;
}
