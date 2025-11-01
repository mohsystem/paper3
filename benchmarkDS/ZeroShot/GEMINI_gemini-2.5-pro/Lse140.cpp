#include <iostream>
#include <string>
#include <sqlite3.h> // Include the SQLite3 C API header

/*
 * To compile and run this code, you need to have the SQLite3 library installed.
 * On Debian/Ubuntu: sudo apt-get install libsqlite3-dev
 * On RedHat/CentOS: sudo yum install sqlite-devel
 * On macOS (with Homebrew): brew install sqlite
 *
 * Compile with: g++ your_file_name.cpp -lsqlite3 -o Lse140
 * Run with: ./Lse140
 */

const char* DB_FILE = "user_database.db";

/**
 * Executes a simple SQL command without expecting results.
 * Used for setup.
 */
void executeSimpleSQL(sqlite3* db, const char* sql) {
    char* errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

/**
 * Sets up the database by creating a users table and inserting sample data.
 */
void setupDatabase() {
    sqlite3* db;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    executeSimpleSQL(db, "DROP TABLE IF EXISTS users;");
    executeSimpleSQL(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT NOT NULL UNIQUE);");
    executeSimpleSQL(db, "INSERT INTO users (username) VALUES ('alice');");
    executeSimpleSQL(db, "INSERT INTO users (username) VALUES ('bob');");
    executeSimpleSQL(db, "INSERT INTO users (username) VALUES ('charlie');");

    sqlite3_close(db);
}


/**
 * Securely checks if a username exists in the database.
 * Uses a prepared statement to prevent SQL injection.
 *
 * @param username The username to check.
 * @return true if the username exists, false otherwise.
 */
bool usernameExists(const std::string& username) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    bool exists = false;
    
    // NOTE: In a real-world application, connection details should come from a secure source.
    int rc = sqlite3_open(DB_FILE, &db);
    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    const char* sql = "SELECT 1 FROM users WHERE username = ?;";
    
    // Prepare the SQL statement. This compiles the SQL query and is the first step
    // in preventing SQL injection.
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    // Bind the username parameter to the '?' placeholder.
    // The value is treated as a literal string, not as part of the SQL command.
    // SQLITE_TRANSIENT tells SQLite to make its own private copy of the data.
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    // Execute the statement and check the result.
    // sqlite3_step() will return SQLITE_ROW if a record is found.
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = true;
    }

    // Finalize the statement to release resources. This is crucial.
    sqlite3_finalize(stmt);
    
    // Close the database connection.
    sqlite3_close(db);
    
    return exists;
}


int main() {
    setupDatabase();

    std::cout << "--- C++ Test Cases ---" << std::endl;

    // Test Case 1: Check for an existing user
    std::string user1 = "alice";
    std::cout << "Does '" << user1 << "' exist? " << (usernameExists(user1) ? "true" : "false") << std::endl; // Expected: true

    // Test Case 2: Check for a non-existent user
    std::string user2 = "david";
    std::cout << "Does '" << user2 << "' exist? " << (usernameExists(user2) ? "true" : "false") << std::endl; // Expected: false

    // Test Case 3: Check for another existing user (case-sensitive)
    std::string user3 = "Bob";
    std::cout << "Does '" << user3 << "' exist? " << (usernameExists(user3) ? "true" : "false") << std::endl; // Expected: false

    // Test Case 4: SQL Injection attempt. Should be handled safely.
    std::string user4 = "' OR '1'='1";
    std::cout << "Does '" << user4 << "' exist? " << (usernameExists(user4) ? "true" : "false") << std::endl; // Expected: false

    // Test Case 5: Check for an empty string
    std::string user5 = "";
    std::cout << "Does '" << user5 << "' exist? " << (usernameExists(user5) ? "true" : "false") << std::endl; // Expected: false

    return 0;
}