
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <memory>
#include <vector>

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
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    std::unique_ptr<Customer> customer = nullptr;
    
    int rc = sqlite3_open("customer.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return nullptr;
    }
    
    const char* sql = "SELECT * FROM customer WHERE customerusername = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    
    if (rc == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, customerUsername.c_str(), -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            customer = std::make_unique<Customer>(
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)),
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)),
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)),
                reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))
            );
        }
    } else {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return customer;
}

void setupTestDatabase() {
    sqlite3* db = nullptr;
    char* errMsg = nullptr;
    
    int rc = sqlite3_open("customer.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }
    
    const char* sql = 
        "DROP TABLE IF EXISTS customer;"
        "CREATE TABLE customer ("
        "customerusername TEXT PRIMARY KEY, "
        "name TEXT, "
        "email TEXT, "
        "phone TEXT);"
        "INSERT INTO customer VALUES ('user1', 'John Doe', 'john@email.com', '123-456-7890');"
        "INSERT INTO customer VALUES ('user2', 'Jane Smith', 'jane@email.com', '098-765-4321');"
        "INSERT INTO customer VALUES ('user3', 'Bob Johnson', 'bob@email.com', '555-555-5555');"
        "INSERT INTO customer VALUES ('user4', 'Alice Brown', 'alice@email.com', '444-444-4444');"
        "INSERT INTO customer VALUES ('user5', 'Charlie Davis', 'charlie@email.com', '333-333-3333');";
    
    rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

int main(int argc, char* argv[]) {
    setupTestDatabase();
    
    std::vector<std::string> testCases = {"user1", "user2", "user3", "user4", "nonexistent"};
    
    if (argc > 1) {
        auto customer = getCustomerByUsername(argv[1]);
        if (customer) {
            std::cout << "Found: " << *customer << std::endl;
        } else {
            std::cout << "Customer not found with username: " << argv[1] << std::endl;
        }
    } else {
        std::cout << "Running test cases:" << std::endl;
        for (const auto& username : testCases) {
            std::cout << "\\nSearching for: " << username << std::endl;
            auto customer = getCustomerByUsername(username);
            if (customer) {
                std::cout << "Found: " << *customer << std::endl;
            } else {
                std::cout << "Customer not found" << std::endl;
            }
        }
    }
    
    return 0;
}
