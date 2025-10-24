#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sqlite3.h"

// Note: To compile this code, you need to link against the SQLite3 library.
// For example: gcc your_file.c -o your_program -lsqlite3

typedef struct {
    int id;
    char username[50];
    char full_name[100];
    char email[100];
} User;

/**
 * Executes a simple SQL command (for setup).
 */
void execute_simple_query(sqlite3* db, const char* sql) {
    char* zErrMsg = NULL;
    int rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
}

/**
 * Sets up the database with a users table and some sample data.
 */
void setup_database(sqlite3* db) {
    const char* createTableSQL = "CREATE TABLE users ("
                                 "id INTEGER PRIMARY KEY, "
                                 "username TEXT NOT NULL UNIQUE, "
                                 "full_name TEXT NOT NULL, "
                                 "email TEXT NOT NULL UNIQUE);";
    execute_simple_query(db, createTableSQL);
    execute_simple_query(db, "INSERT INTO users(username, full_name, email) VALUES('alice', 'Alice Smith', 'alice@example.com');");
    execute_simple_query(db, "INSERT INTO users(username, full_name, email) VALUES('bob', 'Bob Johnson', 'bob@example.com');");
    execute_simple_query(db, "INSERT INTO users(username, full_name, email) VALUES('carol', 'Carol White', 'carol@example.com');");
}

/**
 * Retrieves user information from the database based on the username.
 * Uses a prepared statement to prevent SQL injection.
 *
 * @param db The database connection handle.
 * @param username The username to search for.
 * @param out_user Pointer to a User struct to be filled if a user is found.
 * @return 0 on success (user found), 1 if user not found, -1 on error.
 */
int get_user_info(sqlite3* db, const char* username, User* out_user) {
    const char* query = "SELECT id, username, full_name, email FROM users WHERE username = ?;";
    sqlite3_stmt* stmt = NULL;
    int result_code = 1; // Default to "not found"

    // Prepare the SQL statement
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return -1; // Error
    }

    // Bind the username parameter to the placeholder '?'
    // This is the crucial step to prevent SQL injection
    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1; // Error
    }

    // Execute the statement
    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // A row was found, extract the data and copy it to the output struct
        out_user->id = sqlite3_column_int(stmt, 0);
        strncpy(out_user->username, (const char*)sqlite3_column_text(stmt, 1), sizeof(out_user->username) - 1);
        out_user->username[sizeof(out_user->username) - 1] = '\0'; // Ensure null termination
        
        strncpy(out_user->full_name, (const char*)sqlite3_column_text(stmt, 2), sizeof(out_user->full_name) - 1);
        out_user->full_name[sizeof(out_user->full_name) - 1] = '\0';

        strncpy(out_user->email, (const char*)sqlite3_column_text(stmt, 3), sizeof(out_user->email) - 1);
        out_user->email[sizeof(out_user->email) - 1] = '\0';
        
        result_code = 0; // Success
    } else if (rc != SQLITE_DONE) {
        // An error occurred during execution
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
        result_code = -1; // Error
    }

    // Finalize the statement to free resources
    sqlite3_finalize(stmt);
    return result_code;
}

int main() {
    sqlite3* db;
    int rc = sqlite3_open(":memory:", &db);

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    } else {
        printf("Connected to the in-memory SQLite database.\n");
    }

    setup_database(db);

    // --- Test Cases ---
    const char* test_usernames[] = {
        "alice",                // 1. Valid user
        "bob",                  // 2. Another valid user
        "charlie",              // 3. Non-existent user
        "' OR '1'='1' --",      // 4. SQL injection attempt
        "",                     // 5. Empty string username
        NULL                    // Sentinel to end the array
    };

    for (int i = 0; test_usernames[i] != NULL; ++i) {
        const char* username = test_usernames[i];
        printf("\n--- Searching for user: '%s' ---\n", username);
        
        User found_user;
        int result = get_user_info(db, username, &found_user);
        
        if (result == 0) {
            printf("User found: {id=%d, username='%s', fullName='%s', email='%s'}\n",
                   found_user.id, found_user.username, found_user.full_name, found_user.email);
        } else if (result == 1) {
            printf("User not found.\n");
        } else {
            printf("An error occurred during the search.\n");
        }
    }

    sqlite3_close(db);
    return 0;
}