/*
To compile and run this C++ code, you need to have the SQLite3 development library installed.
On Debian/Ubuntu: sudo apt-get install libsqlite3-dev
On Fedora/CentOS: sudo yum install sqlite-devel
On macOS (using Homebrew): brew install sqlite3

Compile with: g++ your_file_name.cpp -lsqlite3 -o program
Run with: ./program
*/
#include <iostream>
#include <string>
#include <sqlite3.h>

/**
 * Checks if a username exists in the users table.
 *
 * @param db The SQLite3 database connection handle.
 * @param username The username to check.
 * @return true if the username exists, false otherwise.
 */
bool checkUsernameExists(sqlite3* db, const std::string& username) {
    sqlite3_stmt* stmt = nullptr;
    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1;";
    bool exists = false;

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Bind the username parameter to the prepared statement
    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return false;
    }

    // Execute the statement
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // A row was found, so the user exists
        exists = true;
    } else if (rc != SQLITE_DONE) {
        // An error occurred during execution
        std::cerr << "Failed to step statement: " << sqlite3_errmsg(db) << std::endl;
    }

    // Clean up the statement
    sqlite3_finalize(stmt);
    return exists;
}

/**
 * Helper function to execute simple SQL commands and check for errors.
 */
void executeSQL(sqlite3* db, const char* sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}


int main() {
    sqlite3* db = nullptr;

    // Open an in-memory SQLite database
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    }
    std::cout << "In-memory SQLite database connection established." << std::endl;

    // Setup the database
    executeSQL(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT NOT NULL UNIQUE);");
    executeSQL(db, "INSERT INTO users (username) VALUES ('alice');");
    executeSQL(db, "INSERT INTO users (username) VALUES ('bob');");
    executeSQL(db, "INSERT INTO users (username) VALUES ('carol');");
    std::cout << "Database setup complete." << std::endl;

    // --- Test Cases ---
    std::cout << "\n--- Running Test Cases ---" << std::endl;

    // Test Case 1: Username that exists
    std::string user1 = "alice";
    std::cout << "1. Checking for username '" << user1 << "': " 
              << (checkUsernameExists(db, user1) ? "Exists" : "Does not exist") << std::endl;

    // Test Case 2: Username that does not exist
    std::string user2 = "david";
    std::cout << "2. Checking for username '" << user2 << "': " 
              << (checkUsernameExists(db, user2) ? "Exists" : "Does not exist") << std::endl;

    // Test Case 3: Another username that exists
    std::string user3 = "bob";
    std::cout << "3. Checking for username '" << user3 << "': " 
              << (checkUsernameExists(db, user3) ? "Exists" : "Does not exist") << std::endl;

    // Test Case 4: Case-sensitive check (should not exist)
    std::string user4 = "Alice";
    std::cout << "4. Checking for username '" << user4 << "': " 
              << (checkUsernameExists(db, user4) ? "Exists" : "Does not exist") << std::endl;
    
    // Test Case 5: Empty string username
    std::string user5 = "";
    std::cout << "5. Checking for username '" << user5 << "': " 
              << (checkUsernameExists(db, user5) ? "Exists" : "Does not exist") << std::endl;

    // Close the database connection
    sqlite3_close(db);
    std::cout << "\nDatabase connection closed." << std::endl;

    return 0;
}