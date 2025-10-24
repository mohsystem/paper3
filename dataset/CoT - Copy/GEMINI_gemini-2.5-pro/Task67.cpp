#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "sqlite3.h"

// A struct to hold customer data
struct Customer {
    int id;
    std::string username;
    std::string fullName;
    std::string email;
};

// Function to print customer details
void print_customer(const Customer& c) {
    std::cout << "Customer [ID=" << c.id << ", Username=" << c.username 
              << ", FullName=" << c.fullName << ", Email=" << c.email << "]" << std::endl;
}

// Sets up the in-memory database: creates table and inserts sample data.
void setup_database(sqlite3* db) {
    char* err_msg = 0;
    const char* sql = 
        "CREATE TABLE customer ("
        "id INT PRIMARY KEY, "
        "username TEXT NOT NULL UNIQUE, "
        "fullname TEXT, "
        "email TEXT);"
        "INSERT INTO customer VALUES (1, 'jdoe', 'John Doe', 'john.doe@example.com');"
        "INSERT INTO customer VALUES (2, 'asmith', 'Alice Smith', 'alice.smith@example.com');"
        "INSERT INTO customer VALUES (3, 'bwhite', 'Bob White', 'bob.white@example.com');"
        "INSERT INTO customer VALUES (4, 'peterpan', 'Peter Pan', 'peter.pan@example.com');";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL setup error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
    }
}

/**
 * @brief Retrieves a customer by username using a secure prepared statement.
 * @param db The SQLite database connection handle.
 * @param customerUsername The username to search for.
 * @return An std::optional<Customer> which contains a customer if found, otherwise it's empty.
 */
std::optional<Customer> getCustomerByUsername(sqlite3* db, const std::string& customerUsername) {
    sqlite3_stmt* stmt = nullptr;
    // SQL query with a placeholder (?) to prevent SQL injection
    const char* sql = "SELECT id, username, fullname, email FROM customer WHERE username = ?;";

    // Prepare the statement
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    // Bind the username parameter to the placeholder. This is the key security step.
    rc = sqlite3_bind_text(stmt, 1, customerUsername.c_str(), -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt); // Clean up on failure
        return std::nullopt;
    }
    
    // Execute the statement
    rc = sqlite3_step(stmt);
    std::optional<Customer> result = std::nullopt;

    if (rc == SQLITE_ROW) { // A row was found
        Customer c;
        c.id = sqlite3_column_int(stmt, 0);
        c.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        c.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        c.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        result = c;
    } else if (rc != SQLITE_DONE) { // An error occurred
         std::cerr << "Execution failed: " << sqlite3_errmsg(db) << std::endl;
    }

    // Finalize the statement to free resources
    sqlite3_finalize(stmt);
    return result;
}

int main(int argc, char* argv[]) {
    // To compile: g++ your_file_name.cpp -o your_program_name -lsqlite3
    sqlite3* db;
    int rc = sqlite3_open(":memory:", &db); // Use in-memory database

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    setup_database(db);

    if (argc > 1) {
        // If a command-line argument is provided, use it
        std::string username = argv[1];
        std::cout << "Searching for username from command line: " << username << std::endl;
        auto customer = getCustomerByUsername(db, username);
        if (customer) {
            std::cout << "Found: ";
            print_customer(*customer);
        } else {
            std::cout << "Customer with username '" << username << "' not found." << std::endl;
        }
    } else {
        // Otherwise, run predefined test cases
        std::cout << "--- Running Predefined Test Cases ---" << std::endl;
        std::vector<std::string> testUsernames = {
            "jdoe",             // Test Case 1: Existing user
            "asmith",           // Test Case 2: Existing user
            "nonexistent",      // Test Case 3: Non-existing user
            "peterpan",         // Test Case 4: Existing user
            "admin' OR '1'='1"  // Test Case 5: SQL injection attempt
        };

        for (const auto& username : testUsernames) {
            std::cout << "\nSearching for username: " << username << std::endl;
            auto customer = getCustomerByUsername(db, username);
            if (customer) {
                std::cout << "Found: ";
                print_customer(*customer);
            } else {
                std::cout << "Customer with username '" << username << "' not found." << std::endl;
            }
        }
    }
    
    sqlite3_close(db);
    return 0;
}