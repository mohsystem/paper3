/*
To compile and run this C code, you need to have the SQLite3 development library installed.
On Debian/Ubuntu: sudo apt-get install libsqlite3-dev
On Fedora/CentOS: sudo yum install sqlite-devel
On macOS (using Homebrew): brew install sqlite3

Compile with: gcc your_file_name.c -lsqlite3 -o program
Run with: ./program
*/
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

/**
 * Checks if a username exists in the users table.
 *
 * @param db The SQLite3 database connection handle.
 * @param username The username to check.
 * @return 1 if the username exists, 0 otherwise.
 */
int checkUsernameExists(sqlite3* db, const char* username) {
    sqlite3_stmt* stmt = NULL;
    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1;";
    int exists = 0;

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    // Bind the username parameter to the prepared statement
    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    // Execute the statement
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // A row was found, so the user exists
        exists = 1;
    } else if (rc != SQLITE_DONE) {
        // An error occurred during execution
        fprintf(stderr, "Failed to step statement: %s\n", sqlite3_errmsg(db));
    }
    
    // Clean up the statement
    sqlite3_finalize(stmt);
    return exists;
}

/**
 * Helper function to execute simple SQL commands and check for errors.
 */
void executeSQL(sqlite3* db, const char* sql) {
    char* errMsg = NULL;
    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
}


int main() {
    sqlite3* db = NULL;

    // Open an in-memory SQLite database
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    printf("In-memory SQLite database connection established.\n");

    // Setup the database
    executeSQL(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT NOT NULL UNIQUE);");
    executeSQL(db, "INSERT INTO users (username) VALUES ('alice');");
    executeSQL(db, "INSERT INTO users (username) VALUES ('bob');");
    executeSQL(db, "INSERT INTO users (username) VALUES ('carol');");
    printf("Database setup complete.\n");

    // --- Test Cases ---
    printf("\n--- Running Test Cases ---\n");

    // Test Case 1: Username that exists
    const char* user1 = "alice";
    printf("1. Checking for username '%s': %s\n", user1,
           checkUsernameExists(db, user1) ? "Exists" : "Does not exist");

    // Test Case 2: Username that does not exist
    const char* user2 = "david";
    printf("2. Checking for username '%s': %s\n", user2,
           checkUsernameExists(db, user2) ? "Exists" : "Does not exist");

    // Test Case 3: Another username that exists
    const char* user3 = "bob";
    printf("3. Checking for username '%s': %s\n", user3,
           checkUsernameExists(db, user3) ? "Exists" : "Does not exist");
    
    // Test Case 4: Case-sensitive check (should not exist)
    const char* user4 = "Alice";
    printf("4. Checking for username '%s': %s\n", user4,
           checkUsernameExists(db, user4) ? "Exists" : "Does not exist");

    // Test Case 5: Empty string username
    const char* user5 = "";
    printf("5. Checking for username '%s': %s\n", user5,
           checkUsernameExists(db, user5) ? "Exists" : "Does not exist");

    // Close the database connection
    sqlite3_close(db);
    printf("\nDatabase connection closed.\n");

    return 0;
}