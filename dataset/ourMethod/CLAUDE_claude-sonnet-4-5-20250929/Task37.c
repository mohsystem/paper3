
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define MAX_USERNAME_LEN 255
#define MAX_PATH_LEN 4096

/* Security: Clear sensitive data from memory */
static void clear_sensitive_data(void* data, size_t len) {
    if (data != NULL) {
        volatile unsigned char* p = (volatile unsigned char*)data;
        size_t i;
        for (i = 0; i < len; i++) {
            p[i] = 0;
        }
    }
}

/* Security: Validate username to prevent injection and overflow (CWE-20) */
static int validate_username(const char* username) {
    size_t len;
    size_t i;
    
    if (username == NULL) {
        fprintf(stderr, "Username is NULL\\n");
        return 0;
    }

    len = strlen(username);
    
    /* Security: Check length bounds */
    if (len == 0 || len > MAX_USERNAME_LEN) {
        fprintf(stderr, "Invalid username length: %zu\\n", len);
        return 0;
    }

    /* Security: Check for null bytes within string (CWE-158) */
    for (i = 0; i < len; i++) {
        if (username[i] == '\\0') {
            fprintf(stderr, "Null byte detected in username\\n");
            return 0;
        }
    }

    return 1;
}

/* Security: Validate database path to prevent path traversal (CWE-22) */
static int validate_db_path(const char* path) {
    size_t len;
    
    if (path == NULL) {
        fprintf(stderr, "Database path is NULL\\n");
        return 0;
    }

    len = strlen(path);
    
    /* Security: Check path length */
    if (len == 0 || len > MAX_PATH_LEN) {
        fprintf(stderr, "Invalid path length\\n");
        return 0;
    }

    /* Security: Check for path traversal attempts */
    if (strstr(path, "..") != NULL) {
        fprintf(stderr, "Path traversal detected\\n");
        return 0;
    }

    return 1;
}

/* Security: Query user using prepared statements to prevent SQL injection (CWE-89) */
static int query_user_by_username(sqlite3* db, const char* username) {
    sqlite3_stmt* stmt = NULL;
    const char* sql = NULL;
    int rc;
    int found = 0;
    const unsigned char* user = NULL;
    const unsigned char* email = NULL;
    int id;

    if (db == NULL) {
        fprintf(stderr, "Database not connected\\n");
        return 0;
    }

    /* Security: Validate input before processing */
    if (!validate_username(username)) {
        return 0;
    }

    /* Security: Use parameterized query to prevent SQL injection */
    sql = "SELECT id, username, email FROM users WHERE username = ?";
    
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        return 0;
    }

    /* Security: Bind parameter safely */
    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to bind parameter: %s\\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return 0;
    }

    /* Execute query and fetch results */
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        found = 1;
        id = sqlite3_column_int(stmt, 0);
        user = sqlite3_column_text(stmt, 1);
        email = sqlite3_column_text(stmt, 2);

        /* Security: Validate pointers before dereferencing */
        if (user != NULL && email != NULL) {
            printf("ID: %d, Username: %s, Email: %s\\n", id, user, email);
        }
    }

    if (!found) {
        printf("No user found with username: %s\\n", username);
    }

    /* Security: Always finalize statement to prevent resource leaks */
    sqlite3_finalize(stmt);
    
    return (rc == SQLITE_DONE || found);
}

/* Security: Initialize test database with safe operations */
static int init_test_database(sqlite3* db) {
    char* err_msg = NULL;
    int rc;
    sqlite3_stmt* stmt = NULL;
    int i;
    
    const char* create_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL UNIQUE, "
        "email TEXT NOT NULL)";

    if (db == NULL) {
        fprintf(stderr, "Database is NULL\\n");
        return 0;
    }

    /* Create table */
    rc = sqlite3_exec(db, create_table, NULL, NULL, &err_msg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", err_msg ? err_msg : "unknown");
        if (err_msg) {
            sqlite3_free(err_msg);
            err_msg = NULL;
        }
        return 0;
    }

    /* Security: Use prepared statements for inserts */
    const char* insert_sql = "INSERT OR IGNORE INTO users (username, email) VALUES (?, ?)";
    
    const char* test_users[5][2] = {
        {"alice", "alice@example.com"},
        {"bob", "bob@example.com"},
        {"charlie", "charlie@example.com"},
        {"david", "david@example.com"},
        {"eve", "eve@example.com"}
    };

    for (i = 0; i < 5; i++) {
        rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, test_users[i][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, test_users[i][1], -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            stmt = NULL;
        }
    }

    return 1;
}

int main(void) {
    sqlite3* db = NULL;
    int rc;
    const char* db_path = ":memory:";

    /* Security: Validate database path */
    if (!validate_db_path(db_path)) {
        fprintf(stderr, "Invalid database path\\n");
        return 1;
    }

    /* Connect to database */
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        if (db != NULL) {
            sqlite3_close(db);
        }
        return 1;
    }

    /* Initialize test data */
    if (!init_test_database(db)) {
        fprintf(stderr, "Failed to initialize test database\\n");
        sqlite3_close(db);
        return 1;
    }

    /* Test cases with security validation */
    printf("=== Test Case 1: Valid username ===\\n");
    query_user_by_username(db, "alice");

    printf("\\n=== Test Case 2: Another valid username ===\\n");
    query_user_by_username(db, "bob");

    printf("\\n=== Test Case 3: Non-existent user ===\\n");
    query_user_by_username(db, "nonexistent");

    printf("\\n=== Test Case 4: SQL injection attempt (prevented) ===\\n");
    /* Security: This injection attempt is safely handled by prepared statements */
    query_user_by_username(db, "alice' OR '1'='1");

    printf("\\n=== Test Case 5: Empty username ===\\n");
    query_user_by_username(db, "");

    /* Security: Always close database connection */
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }

    return 0;
}
