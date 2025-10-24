#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define MAX_USERNAME_LENGTH 64
#define MAX_BUFFER_SIZE 256

// Function to set up the database with a users table and sample data
int setup_database(sqlite3* db) {
    char* err_msg = NULL;
    const char* sql =
        "DROP TABLE IF EXISTS users;"
        "CREATE TABLE users(id INTEGER PRIMARY KEY, username TEXT NOT NULL UNIQUE, full_name TEXT, email TEXT);"
        "INSERT INTO users (username, full_name, email) VALUES ('alice', 'Alice Smith', 'alice@example.com');"
        "INSERT INTO users (username, full_name, email) VALUES ('bob', 'Bob Johnson', 'bob@example.com');";

    int rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error in setup: %s\n", err_msg);
        sqlite3_free(err_msg);
        return rc;
    }
    return SQLITE_OK;
}

// Function to retrieve user information based on username
int get_user_info(sqlite3* db, const char* username) {
    // Rule #1: Ensure all input is validated.
    if (username == NULL || username[0] == '\0') {
        fprintf(stderr, "Error: Username cannot be null or empty.\n");
        return -1;
    }
    
    // C Security: Check buffer length before use.
    size_t username_len = strlen(username);
    if (username_len > MAX_USERNAME_LENGTH) {
        fprintf(stderr, "Error: Username is too long.\n");
        return -1;
    }

    // Rule #7: Use constant format strings.
    const char* sql = "SELECT id, full_name, email FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = NULL;
    int rc = 0;
    int found_count = 0;

    // Use prepared statements to prevent SQL injection (CWE-89)
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return rc;
    }

    // Bind the username parameter
    rc = sqlite3_bind_text(stmt, 1, username, (int)username_len, SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt); // Cleanup
        return rc;
    }

    printf("Querying for user: '%s'\n", username);
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        found_count++;
        int id = sqlite3_column_int(stmt, 0);
        const unsigned char* full_name = sqlite3_column_text(stmt, 1);
        const unsigned char* email = sqlite3_column_text(stmt, 2);

        printf("Found user -> ID: %d, Name: %s, Email: %s\n",
               id,
               full_name ? (const char*)full_name : "N/A",
               email ? (const char*)email : "N/A");
    }

    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Failed to step statement: %s\n", sqlite3_errmsg(db));
    } else if (found_count == 0) {
        printf("No user found.\n");
    }

    // C Security: All allocated memory freed exactly once
    sqlite3_finalize(stmt);
    return SQLITE_OK;
}

void run_test_case(sqlite3* db, const char* username) {
    printf("--- Testing with username: \"%s\" ---\n", username ? username : "NULL");
    int rc = get_user_info(db, username);
    if (rc != SQLITE_OK && rc != -1) {
        fprintf(stderr, "Test case failed with SQLite error code: %d\n", rc);
    }
    printf("\n");
}

int main(void) {
    // Rule #6: Avoid hardcoding credentials. Using an in-memory database for this example.
    // For a file-based DB, use environment variables:
    // const char* db_path = getenv("DB_PATH");
    // if (db_path == NULL) { db_path = "users.db"; }
    const char* db_path = ":memory:";
    
    sqlite3* db = NULL;
    int rc = sqlite3_open(db_path, &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    if (setup_database(db) != SQLITE_OK) {
        sqlite3_close(db);
        return 1;
    }

    // --- 5 Test Cases ---
    
    // 1. Valid user who exists
    run_test_case(db, "alice");

    // 2. Valid user who does not exist
    run_test_case(db, "charlie");

    // 3. Potential SQL injection attempt
    run_test_case(db, "admin' OR 1=1; --");
    
    // 4. Empty username (should be rejected by validation)
    run_test_case(db, "");

    // 5. Username longer than max length (should be rejected by validation)
    char long_username[MAX_USERNAME_LENGTH + 2];
    memset(long_username, 'a', MAX_USERNAME_LENGTH + 1);
    long_username[MAX_USERNAME_LENGTH + 1] = '\0';
    run_test_case(db, long_username);
    
    // C Security: All allocated memory freed
    sqlite3_close(db);

    return 0;
}