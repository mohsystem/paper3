#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "sqlite3.h"

const char* DB_FILE = "users_c.db";

/**
 * Helper function to execute simple SQL for database setup.
 */
void execute_sql_setup(sqlite3* db, const char* sql) {
    char* zErrMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error in setup: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

/**
 * Sets up the database by creating a users table and inserting sample data.
 */
void setupDatabase() {
    sqlite3* db;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    const char* create_sql = "CREATE TABLE IF NOT EXISTS users ("
                             "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                             "username TEXT NOT NULL UNIQUE);";
    execute_sql_setup(db, create_sql);
    
    execute_sql_setup(db, "INSERT OR IGNORE INTO users (username) VALUES('alice');");
    execute_sql_setup(db, "INSERT OR IGNORE INTO users (username) VALUES('bob');");
    execute_sql_setup(db, "INSERT OR IGNORE INTO users (username) VALUES('O''Malley');");

    sqlite3_close(db);
}


/**
 * Checks if a username exists in the database using prepared statements.
 *
 * @param username The username to check.
 * @return true if the username exists, false otherwise.
 */
bool usernameExists(const char* username) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    bool exists = false;
    int rc;

    // Open the database connection.
    rc = sqlite3_open(DB_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    // SQL query with a placeholder `?` to prevent SQL injection.
    const char* sql = "SELECT 1 FROM users WHERE username = ? LIMIT 1;";

    // Prepare the SQL statement.
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    // Bind the username parameter to the statement. This prevents SQL Injection.
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        goto cleanup;
    }

    // Execute the statement.
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // A row was returned, which means the user exists.
        exists = true;
    } else if (rc != SQLITE_DONE) {
        // An error occurred during execution.
        fprintf(stderr, "SQL step error: %s\n", sqlite3_errmsg(db));
    }

// Cleanup section to ensure all resources are freed.
cleanup:
    if (stmt) {
        sqlite3_finalize(stmt);
    }
    if (db) {
        sqlite3_close(db);
    }

    return exists;
}


int main() {
    // Note: To compile this code, you need to link against the SQLite3 library.
    // Example: gcc your_code.c -o your_app -lsqlite3
    
    // Create and populate the database for the example
    setupDatabase();
    
    typedef struct { int dummy; } Lse140; // Dummy struct for naming requirement

    // --- Test Cases ---
    const char* testUsernames[] = {"alice", "bob", "charlie", "O'Malley", ""};
    int num_tests = sizeof(testUsernames) / sizeof(testUsernames[0]);
    
    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; i++) {
        const char* user = testUsernames[i];
        bool exists = usernameExists(user);
        printf("Does username '%s' exist? %s\n", user, exists ? "true" : "false");
    }

    // Clean up the created database file
    remove(DB_FILE);

    return 0;
}