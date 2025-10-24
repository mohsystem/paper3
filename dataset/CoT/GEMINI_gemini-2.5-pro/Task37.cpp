#include <iostream>
#include <string>
#include <vector>
#include <optional>
#include "sqlite3.h"

// Note: To compile this code, you need to link against the SQLite3 library.
// For example: g++ your_file.cpp -o your_program -lsqlite3

struct User {
    int id;
    std::string username;
    std::string fullName;
    std::string email;
};

void printUser(const User& user) {
    std::cout << "User found: {id=" << user.id
              << ", username='" << user.username
              << "', fullName='" << user.fullName
              << "', email='" << user.email << "'}" << std::endl;
}

/**
 * Executes a simple SQL command (for setup).
 */
void executeSimpleQuery(sqlite3* db, const char* sql) {
    char* zErrMsg = nullptr;
    int rc = sqlite3_exec(db, sql, nullptr, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
}

/**
 * Sets up the database with a users table and some sample data.
 */
void setupDatabase(sqlite3* db) {
    const char* createTableSQL = "CREATE TABLE users ("
                                 "id INTEGER PRIMARY KEY, "
                                 "username TEXT NOT NULL UNIQUE, "
                                 "full_name TEXT NOT NULL, "
                                 "email TEXT NOT NULL UNIQUE);";
    executeSimpleQuery(db, createTableSQL);
    executeSimpleQuery(db, "INSERT INTO users(username, full_name, email) VALUES('alice', 'Alice Smith', 'alice@example.com');");
    executeSimpleQuery(db, "INSERT INTO users(username, full_name, email) VALUES('bob', 'Bob Johnson', 'bob@example.com');");
    executeSimpleQuery(db, "INSERT INTO users(username, full_name, email) VALUES('carol', 'Carol White', 'carol@example.com');");
}


/**
 * Retrieves user information from the database based on the username.
 * Uses a prepared statement to prevent SQL injection.
 * @param db The database connection handle.
 * @param username The username to search for.
 * @return An optional User struct. The optional will be empty if not found or on error.
 */
std::optional<User> getUserInfo(sqlite3* db, const std::string& username) {
    const char* query = "SELECT id, username, full_name, email FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = nullptr;
    std::optional<User> result;

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return std::nullopt;
    }

    // Bind the username parameter to the placeholder '?'
    // This is the crucial step to prevent SQL injection
    if (sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC) != SQLITE_OK) {
        std::cerr << "Failed to bind parameter: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return std::nullopt;
    }

    // Execute the statement
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // A row was found, extract the data
        User user;
        user.id = sqlite3_column_int(stmt, 0);
        user.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        user.fullName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        user.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        result = user;
    } else if (rc != SQLITE_DONE) {
        // An error occurred during execution
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }
    
    // Finalize the statement to free resources
    sqlite3_finalize(stmt);

    return result;
}

int main() {
    sqlite3* db;
    int rc = sqlite3_open(":memory:", &db);

    if (rc) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        return 1;
    } else {
        std::cout << "Connected to the in-memory SQLite database." << std::endl;
    }

    setupDatabase(db);
    
    // --- Test Cases ---
    std::vector<std::string> testUsernames = {
        "alice",                // 1. Valid user
        "bob",                  // 2. Another valid user
        "charlie",              // 3. Non-existent user
        "' OR '1'='1' --",      // 4. SQL injection attempt
        ""                      // 5. Empty string username
    };

    for (const auto& username : testUsernames) {
        std::cout << "\n--- Searching for user: '" << username << "' ---" << std::endl;
        auto foundUser = getUserInfo(db, username);
        if (foundUser) {
            printUser(*foundUser);
        } else {
            std::cout << "User not found." << std::endl;
        }
    }
    
    sqlite3_close(db);
    return 0;
}