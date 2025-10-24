// Note: To compile and run this code, you need to have the SQLite3 development library installed.
// Example compile command:
// gcc task37.c -o task37 -lsqlite3
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

const char* DB_FILE = "user_database_c.db";

/**
 * A callback function for sqlite3_exec, used during setup.
 */
static int callback(void* NotUsed, int argc, char** argv, char** azColName) {
    return 0;
}

/**
 * Sets up the database by creating a table and inserting sample data.
 */
void setupDatabase(sqlite3* db) {
    char* zErrMsg = 0;
    const char* sql = "DROP TABLE IF EXISTS users;"
                      "CREATE TABLE users ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "username TEXT NOT NULL UNIQUE,"
                      "full_name TEXT NOT NULL,"
                      "email TEXT NOT NULL);"
                      "INSERT INTO users (username, full_name, email) VALUES "
                      "('alice', 'Alice Smith', 'alice@example.com'),"
                      "('bob', 'Bob Johnson', 'bob@example.com'),"
                      "('admin', 'Administrator', 'admin@example.org');";

    if (sqlite3_exec(db, sql, callback, 0, &zErrMsg) != SQLITE_OK) {
        fprintf(stderr, "SQL error during setup: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    } else {
        printf("Database setup successful.\n");
    }
}

/**
 * Retrieves user information using a secure prepared statement.
 */
void getUserInfo(sqlite3* db, const char* username) {
    sqlite3_stmt* stmt = NULL;
    const char* sql = "SELECT id, username, full_name, email FROM users WHERE username = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC) != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return;
    }

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        printf("Result: User found:\n");
        printf("  ID: %d\n", sqlite3_column_int(stmt, 0));
        printf("  Username: %s\n", sqlite3_column_text(stmt, 1));
        printf("  Full Name: %s\n", sqlite3_column_text(stmt, 2));
        printf("  Email: %s\n", sqlite3_column_text(stmt, 3));
    } else if (rc == SQLITE_DONE) {
        printf("Result: User not found.\n");
    } else {
        fprintf(stderr, "Execution failed: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

int main() {
    sqlite3* db;
    
    if (sqlite3_open(DB_FILE, &db)) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    
    setupDatabase(db);
    
    printf("\n--- Running 5 Test Cases ---\n");
    
    const char* testUsernames[] = {
        "alice",
        "bob",
        "charlie",
        "admin",
        "' OR '1'='1"
    };
    int num_tests = sizeof(testUsernames) / sizeof(testUsernames[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("\n[Test Case %d]\n", i + 1);
        printf("Searching for user: '%s'\n", testUsernames[i]);
        getUserInfo(db, testUsernames[i]);
    }

    sqlite3_close(db);
    remove(DB_FILE);

    return 0;
}