#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "sqlite3.h"

// Note: To compile this code, you need to link against the SQLite3 library.
// Example: g++ your_source_file.cpp -o your_executable -lsqlite3

struct Customer {
    std::string username;
    std::string firstName;
    std::string lastName;
    std::string email;
};

// Callback for database setup, not used for data retrieval but can be useful for exec
static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    return 0;
}

void setupDatabase(sqlite3* db) {
    char* zErrMsg = 0;
    
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS customer ("
        "customer_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "customerusername TEXT NOT NULL UNIQUE,"
        "first_name TEXT NOT NULL,"
        "last_name TEXT NOT NULL,"
        "email TEXT NOT NULL UNIQUE);";

    int rc = sqlite3_exec(db, createTableSQL, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (CREATE): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    
    // Clear the table for a fresh run
    sqlite3_exec(db, "DELETE FROM customer;", callback, 0, &zErrMsg);

    const char* insertSQL = 
        "INSERT INTO customer(customerusername, first_name, last_name, email) VALUES"
        "('jdoe', 'John', 'Doe', 'john.doe@example.com'),"
        "('asmith', 'Anna', 'Smith', 'anna.smith@example.com'),"
        "('sjobs', 'Steve', 'Jobs', 's.jobs@example.com'),"
        "('bgates', 'Bill', 'Gates', 'bill.g@example.com');";
    
    rc = sqlite3_exec(db, insertSQL, callback, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error (INSERT): " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
}

/**
 * Retrieves a customer from the database by username using a prepared statement.
 * @param db The SQLite database connection handle.
 * @param username The username to search for.
 * @return An std::optional<Customer> containing the customer if found, otherwise empty.
 */
std::optional<Customer> getCustomerByUsername(sqlite3* db, const std::string& username) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT customerusername, first_name, last_name, email FROM customer WHERE customerusername = ?;";
    
    // Prepare the SQL statement. This compiles the SQL and checks for syntax errors.
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    // Bind the username parameter to the '?' placeholder.
    // This securely handles the input, preventing SQL injection.
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    std::optional<Customer> customer;

    if (rc == SQLITE_ROW) { // A row was found
        Customer c;
        c.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        c.firstName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        c.lastName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        c.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        customer = c;
    } else if (rc != SQLITE_DONE) {
        std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
    }
    
    // Finalize the statement to release resources.
    sqlite3_finalize(stmt);

    return customer;
}

void printCustomer(const Customer& c) {
    std::cout << "Found: Customer{username='" << c.username << "', firstName='" << c.firstName
              << "', lastName='" << c.lastName << "', email='" << c.email << "'}" << std::endl;
}

int main(int argc, char* argv[]) {
    sqlite3* db;
    int rc = sqlite3_open("customers.db", &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    setupDatabase(db);
    
    std::vector<std::string> testUsernames;

    if (argc > 1) {
        std::cout << "--- Running with provided command-line argument ---" << std::endl;
        testUsernames.push_back(argv[1]);
    } else {
        std::cout << "--- Running 5 built-in test cases ---" << std::endl;
        testUsernames.push_back("jdoe");
        testUsernames.push_back("asmith");
        testUsernames.push_back("nonexistent");
        testUsernames.push_back("sjobs");
        testUsernames.push_back("' OR 1=1; --");
    }

    for (const auto& username : testUsernames) {
        std::cout << "\nSearching for username: '" << username << "'" << std::endl;
        auto cust = getCustomerByUsername(db, username);
        if (cust) {
            printCustomer(*cust);
        } else {
            std::cout << "Result: Customer not found." << std::endl;
        }
    }
    
    sqlite3_close(db);
    return 0;
}