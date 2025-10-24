#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "sqlite3.h"

/*
Note: To compile and run this C++ code, you need the SQLite3 library.
1. Install SQLite3 development libraries (e.g., `sudo apt-get install libsqlite3-dev` on Debian/Ubuntu).
2. Compile: g++ -std=c++17 -o task67_cpp Task67.cpp -lsqlite3
3. Run: ./task67_cpp [optional_username]
*/

const char* DB_NAME = "customers.db";

// Struct to hold customer data
struct Customer {
    int id;
    std::string username;
    std::string firstName;
    std::string lastName;
    std::string email;
};

// Function to print customer details
void printCustomer(const Customer& cust) {
    std::cout << "Customer{id=" << cust.id
              << ", username='" << cust.username << "'"
              << ", firstName='" << cust.firstName << "'"
              << ", lastName='" << cust.lastName << "'"
              << ", email='" << cust.email << "'}" << std::endl;
}

// Sets up the database, creates the table, and inserts sample data
void setupDatabase() {
    sqlite3* db;
    char* zErrMsg = 0;

    if (sqlite3_open(DB_NAME, &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    const char* sql = 
        "CREATE TABLE IF NOT EXISTS customer ("
        "customerid INTEGER PRIMARY KEY AUTOINCREMENT, "
        "customerusername TEXT NOT NULL UNIQUE, "
        "firstname TEXT, "
        "lastname TEXT, "
        "email TEXT);"

        "INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('jdoe', 'John', 'Doe', 'john.doe@email.com');"
        "INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('asmith', 'Alice', 'Smith', 'alice.s@web.com');"
        "INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('bwhite', 'Bob', 'White', 'bwhite@mail.org');"
        "INSERT OR IGNORE INTO customer(customerusername, firstname, lastname, email) VALUES('cjones', 'Carol', 'Jones', 'carolj@email.com');";

    if (sqlite3_exec(db, sql, 0, 0, &zErrMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    
    sqlite3_close(db);
}

/**
 * Retrieves customer information based on username.
 * @param customerUsername The username to search for.
 * @return An std::optional<Customer> which contains a customer if found.
 */
std::optional<Customer> getCustomerByUsername(const std::string& customerUsername) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    std::optional<Customer> result;

    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    const char* sql = "SELECT customerid, customerusername, firstname, lastname, email FROM customer WHERE customerusername = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, customerUsername.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            Customer cust;
            cust.id = sqlite3_column_int(stmt, 0);
            cust.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            cust.firstName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            cust.lastName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            cust.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            result = cust;
        }
    } else {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int main(int argc, char* argv[]) {
    setupDatabase();

    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    std::vector<std::string> testUsernames = {"asmith", "jdoe", "nonexistent", "bwhite", "imnotreal"};
    for (int i = 0; i < testUsernames.size(); ++i) {
        std::cout << "\nTest Case " << (i + 1) << ": Searching for '" << testUsernames[i] << "'" << std::endl;
        auto custOpt = getCustomerByUsername(testUsernames[i]);
        if (custOpt) {
            std::cout << "Found: ";
            printCustomer(*custOpt);
        } else {
            std::cout << "Customer not found." << std::endl;
        }
    }
    std::cout << "----------------------------\n" << std::endl;

    if (argc > 1) {
        std::string usernameFromArg = argv[1];
        std::cout << "--- Running with Command-Line Argument ---" << std::endl;
        std::cout << "Searching for '" << usernameFromArg << "'" << std::endl;
        auto custOpt = getCustomerByUsername(usernameFromArg);
        if (custOpt) {
            std::cout << "Found: ";
            printCustomer(*custOpt);
        } else {
            std::cout << "Customer not found." << std::endl;
        }
        std::cout << "------------------------------------------" << std::endl;
    }
    return 0;
}