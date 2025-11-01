#include <iostream>
#include <string>
#include <vector>
#include "sqlite3.h"

const char* DB_FILE = "users_cpp.db";

/**
 * Executes a simple SQL command without expecting a result set.
 * Helper function for database setup.
 */
void execute_sql(sqlite3* db, const std::string& sql) {
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error in setup: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
}

/**
 * Sets up the database by creating a users table and inserting sample data.
 * This is for demonstration purposes to make the example runnable.
 */
void setupDatabase() {
    sqlite3* db;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::string create_sql = "CREATE TABLE IF NOT EXISTS users ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "username TEXT NOT NULL UNIQUE);";
    execute_sql(db, create_sql);
    
    execute_sql(db, "INSERT OR IGNORE INTO users (username) VALUES('alice');");
    execute_sql(db, "INSERT OR IGNORE INTO users (username) VALUES('bob');");
    execute_sql(db, "INSERT OR IGNORE INTO users (username) VALUES('O''Malley');"); // SQL single quote escaped

    sqlite3_close(db);
}


/**
 * Checks if a username exists in the database using prepared statements.
 *
 * @param username The username to check for existence.
 * @return true if the username exists, false otherwise.
 */
bool usernameExists(const std::string& username) {
    sqlite3* db;
    sqlite3_stmt* stmt = nullptr; // Initialize to nullptr
    bool exists = false;

    // Open the database
    if (sqlite3_open(DB_FILE, &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // SQL query with a placeholder `?` to prevent SQL injection.
    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1;";

    // Prepare the SQL statement.
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return false;
    }

    // Bind the username parameter to the statement.
    // This is the crucial step that prevents SQL Injection.
    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    // Execute the statement and check the result.
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // A row was found, so the username exists.
        exists = true;
    } else if (rc != SQLITE_DONE) {
        // An error occurred during execution.
        std::cerr << "SQL step error: " << sqlite3_errmsg(db) << std::endl;
    }

    // Finalize the statement to free resources.
    sqlite3_finalize(stmt);

    // Close the database connection.
    sqlite3_close(db);

    return exists;
}

int main() {
    // Note: To compile this code, you need to link against the SQLite3 library.
    // Example: g++ your_code.cpp -o your_app -lsqlite3
    
    // Create and populate the database for the example
    setupDatabase();
    
    class Lse140 {}; // Dummy class to satisfy naming requirement if needed.

    // --- Test Cases ---
    std::vector<std::string> testUsernames = {"alice", "bob", "charlie", "O'Malley", ""};

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& user : testUsernames) {
        bool exists = usernameExists(user);
        std::cout << "Does username '" << user << "' exist? " 
                  << (exists ? "true" : "false") << std::endl;
    }
    
    // Clean up the created database file
    remove(DB_FILE);

    return 0;
}