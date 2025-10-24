#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <regex>
#include "sqlite3.h"

// To compile and run this code, you need to link against the SQLite3 library.
// Example: g++ -std=c++17 Task67.cpp -o Task67 -lsqlite3

struct Customer {
    int id;
    std::string username;
    std::string firstName;
    std::string lastName;
    std::string email;
};

// Simple username validation: Alphanumeric, 3-20 characters.
bool isValidUsername(const std::string& username) {
    static const std::regex pattern("^[a-zA-Z0-9]{3,20}$");
    return std::regex_match(username, pattern);
}

void printCustomer(const Customer& customer) {
    std::cout << "Customer{id=" << customer.id
              << ", username='" << customer.username << '\''
              << ", firstName='" << customer.firstName << '\''
              << ", lastName='" << customer.lastName << '\''
              << ", email='" << customer.email << "'}" << std::endl;
}

void setupDatabase(sqlite3* db) {
    char* errMsg = nullptr;
    const char* createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS customer (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL UNIQUE,
            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,
            email TEXT NOT NULL UNIQUE
        );
    )";

    const char* insertSQLs[] = {
        "INSERT OR IGNORE INTO customer(username, first_name, last_name, email) VALUES('jdoe', 'John', 'Doe', 'john.doe@example.com');",
        "INSERT OR IGNORE INTO customer(username, first_name, last_name, email) VALUES('asmith', 'Alice', 'Smith', 'alice.smith@example.com');",
        "INSERT OR IGNORE INTO customer(username, first_name, last_name, email) VALUES('bwilliams', 'Bob', 'Williams', 'bob.williams@example.com');",
        "INSERT OR IGNORE INTO customer(username, first_name, last_name, email) VALUES('mjones', 'Mary', 'Jones', 'mary.jones@example.com');",
        nullptr
    };

    if (sqlite3_exec(db, createTableSQL, 0, 0, &errMsg) != SQLITE_OK) {
        std::string err = "SQL error (CREATE): " + std::string(errMsg);
        sqlite3_free(errMsg);
        throw std::runtime_error(err);
    }
    
    for (int i = 0; insertSQLs[i] != nullptr; ++i) {
        if (sqlite3_exec(db, insertSQLs[i], 0, 0, &errMsg) != SQLITE_OK) {
            std::string err = "SQL error (INSERT): " + std::string(errMsg);
            sqlite3_free(errMsg);
            throw std::runtime_error(err);
        }
    }
}

std::optional<Customer> getCustomerByUsername(sqlite3* db, const std::string& username) {
    if (!isValidUsername(username)) {
        std::cerr << "Invalid username format provided." << std::endl;
        return std::nullopt;
    }

    const char* sql = "SELECT id, username, first_name, last_name, email FROM customer WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;
    std::optional<Customer> result = std::nullopt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        Customer c;
        c.id = sqlite3_column_int(stmt, 0);
        c.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        c.firstName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        c.lastName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        c.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        result = c;
    } else if (rc != SQLITE_DONE) {
        std::cerr << "Failed to step statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return result;
}

int main(int argc, char* argv[]) {
    sqlite3* db;
    if (sqlite3_open(":memory:", &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }

    try {
        setupDatabase(db);
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        sqlite3_close(db);
        return 1;
    }

    if (argc > 1) {
        std::cout << "Searching for username from command line argument: " << argv[1] << std::endl;
        auto customer = getCustomerByUsername(db, argv[1]);
        if (customer) {
            std::cout << "Found: ";
            printCustomer(*customer);
        } else {
            std::cout << "Customer not found." << std::endl;
        }
    } else {
        std::cout << "Running test cases..." << std::endl;
        std::vector<std::string> testUsernames = {"jdoe", "asmith", "bwilliams", "enonexistent", "mjones"};
        for (size_t i = 0; i < testUsernames.size(); ++i) {
            std::cout << "\n--- Test Case " << (i + 1) << ": Searching for username '" << testUsernames[i] << "' ---" << std::endl;
            auto customer = getCustomerByUsername(db, testUsernames[i]);
            if (customer) {
                std::cout << "Found: ";
                printCustomer(*customer);
            } else {
                std::cout << "Customer not found." << std::endl;
            }
        }
    }

    sqlite3_close(db);
    return 0;
}