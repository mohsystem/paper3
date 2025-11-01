#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <sqlite3.h>

/*
 * To compile and run this code, you need to have the SQLite3 library installed.
 * On Debian/Ubuntu: sudo apt-get install libsqlite3-dev
 * On RedHat/CentOS: sudo yum install sqlite-devel
 * On macOS (with Homebrew): brew install sqlite
 *
 * Compile with: gcc your_file_name.c -lsqlite3 -o lse140_c
 * Run with: ./lse140_c
 */

const char* DB_FILE = "user_database.db";

/**
 * Helper to execute simple setup SQL.
 */
void execute_simple_sql(sqlite3* db, const char* sql) {
    char* err_msg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

/**
 * Sets up the database by creating a users table and inserting sample data.
 */
void setup_database() {
    sqlite3* db;
    int rc = sqlite3_open(DB_FILE, &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return;
    }

    execute_simple_sql(db, "DROP TABLE IF EXISTS users;");
    execute_simple_sql(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT NOT NULL UNIQUE);");
    execute_simple_sql(db, "INSERT INTO users (username) VALUES ('alice');");
    execute_simple_sql(db, "INSERT INTO users (username) VALUES ('bob');");
    execute_simple_sql(db, "INSERT INTO users (username) VALUES ('charlie');");

    sqlite3_close(db);
}

/**
 * Securely checks if a username exists in the database.
 * Uses a prepared statement to prevent SQL injection.
 *
 * @param username The username to check.
 * @return true if the username exists, false otherwise.
 */
bool username_exists(const char* username) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    bool exists = false;

    // NOTE: In a real-world application, connection details should come from a secure source.
    int rc = sqlite3_open(DB_FILE, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return false;
    }

    const char* sql = "SELECT 1 FROM users WHERE username = ?;";

    // Prepare the SQL statement. This is the first step in preventing SQL injection.
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return false;
    }

    // Bind the username parameter to the '?' placeholder. The value is treated as
    // a literal string, not as part of the SQL command.
    // SQLITE_TRANSIENT tells SQLite to make its own private copy of the data.
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);

    // Execute the statement. sqlite3_step() will return SQLITE_ROW if a record is found.
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
    setup_database();

    printf("--- C Test Cases ---\n");

    // Test Case 1: Check for an existing user
    const char* user1 = "alice";
    printf("Does '%s' exist? %s\n", user1, username_exists(user1) ? "true" : "false"); // Expected: true

    // Test Case 2: Check for a non-existent user
    const char* user2 = "david";
    printf("Does '%s' exist? %s\n", user2, username_exists(user2) ? "true" : "false"); // Expected: false

    // Test Case 3: Check for another existing user (case-sensitive)
    const char* user3 = "Bob";
    printf("Does '%s' exist? %s\n", user3, username_exists(user3) ? "true" : "false"); // Expected: false

    // Test Case 4: SQL Injection attempt. Should be handled safely.
    const char* user4 = "' OR '1'='1";
    printf("Does '%s' exist? %s\n", user4, username_exists(user4) ? "true" : "false"); // Expected: false

    // Test Case 5: Check for an empty string
    const char* user5 = "";
    printf("Does '%s' exist? %s\n", user5, username_exists(user5) ? "true" : "false"); // Expected: false

    return 0;
}