
#include <iostream>
#include <string>
#include <memory>
#include <sqlite3.h>

class Customer {
public:
    std::string username;
    std::string name;
    std::string email;
    std::string phone;
    
    Customer(const std::string& username, const std::string& name, 
             const std::string& email, const std::string& phone)
        : username(username), name(name), email(email), phone(phone) {}
    
    friend std::ostream& operator<<(std::ostream& os, const Customer& c) {
        os << "Customer{username='" << c.username << "', name='" << c.name 
           << "', email='" << c.email << "', phone='" << c.phone << "'}";
        return os;
    }
};

std::unique_ptr<Customer> getCustomerByUsername(const std::string& customerUsername) {
    if (customerUsername.empty()) {
        return nullptr;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    const char* dbPath = "customer.db";
    
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        std::cerr << "Database error: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    
    const char* query = "SELECT username, name, email, phone FROM customer WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Query preparation error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return nullptr;
    }
    
    sqlite3_bind_text(stmt, 1, customerUsername.c_str(), -1, SQLITE_TRANSIENT);
    
    std::unique_ptr<Customer> customer = nullptr;
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        customer = std::make_unique<Customer>(
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
            reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))
        );
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return customer;
}

void initializeDatabase() {
    sqlite3* db;
    char* errMsg = nullptr;
    
    if (sqlite3_open("customer.db", &db) != SQLITE_OK) {
        std::cerr << "Database initialization error" << std::endl;
        return;
    }
    
    const char* createTable = 
        "CREATE TABLE IF NOT EXISTS customer ("
        "username TEXT PRIMARY KEY, "
        "name TEXT, "
        "email TEXT, "
        "phone TEXT)";
    
    sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg);
    sqlite3_exec(db, "DELETE FROM customer", nullptr, nullptr, &errMsg);
    
    sqlite3_exec(db, "INSERT INTO customer VALUES ('john_doe', 'John Doe', 'john@example.com', '555-0101')", nullptr, nullptr, &errMsg);
    sqlite3_exec(db, "INSERT INTO customer VALUES ('jane_smith', 'Jane Smith', 'jane@example.com', '555-0102')", nullptr, nullptr, &errMsg);
    sqlite3_exec(db, "INSERT INTO customer VALUES ('bob_wilson', 'Bob Wilson', 'bob@example.com', '555-0103')", nullptr, nullptr, &errMsg);
    sqlite3_exec(db, "INSERT INTO customer VALUES ('alice_brown', 'Alice Brown', 'alice@example.com', '555-0104')", nullptr, nullptr, &errMsg);
    sqlite3_exec(db, "INSERT INTO customer VALUES ('charlie_davis', 'Charlie Davis', 'charlie@example.com', '555-0105')", nullptr, nullptr, &errMsg);
    
    sqlite3_close(db);
}

int main(int argc, char* argv[]) {
    initializeDatabase();
    
    std::string testCases[] = {"john_doe", "jane_smith", "bob_wilson", "nonexistent_user", ""};
    
    if (argc > 1) {
        auto customer = getCustomerByUsername(argv[1]);
        if (customer) {
            std::cout << "Found: " << *customer << std::endl;
        } else {
            std::cout << "Customer not found: " << argv[1] << std::endl;
        }
    } else {
        std::cout << "Running test cases:" << std::endl;
        for (const auto& username : testCases) {
            std::cout << "\\nSearching for: '" << username << "'" << std::endl;
            auto customer = getCustomerByUsername(username);
            if (customer) {
                std::cout << "Result: " << *customer << std::endl;
            } else {
                std::cout << "Result: Customer not found" << std::endl;
            }
        }
    }
    
    return 0;
}
