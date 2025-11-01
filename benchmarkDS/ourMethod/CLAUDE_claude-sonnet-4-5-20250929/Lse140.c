
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <stdbool.h>

/* Rules#2: Input validation function */
bool validateUsername(const char* username) {
    /* Check for NULL pointer */
    if (username == NULL) {
        return false;
    }
    
    /* Check if empty */
    size_t len = strnlen(username, 256);
    if (len == 0) {
        return false;
    }
    
    /* Limit length to prevent DoS - Rules#2 */
    if (len > 255) {
        return false;
    }
    
    return true;
}

/* Check if username exists in database
 * Returns true if exists, false otherwise
 */
bool checkUsernameExists(const char* username) {
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    bool exists = false;
    int rc;
    
    /* Rules#2: Validate input before processing */
    if (!validateUsername(username)) {
        return false;
    }
    
    /* Rules#3: Load database path from environment, not hard coded */
    const char* dbPath = getenv("DB_PATH");
    if (dbPath == NULL) {
        dbPath = "database.db";
    }
    
    /* Open database connection */
    rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        /* Rules#11: Don't leak internal details */\n        fprintf(stderr, "Database operation failed\\n");\n        if (db != NULL) {\n            sqlite3_close(db);\n        }\n        return false;\n    }\n    \n    /* Rules#32: Use parameterized queries to prevent SQL injection */\n    const char* sql = "SELECT COUNT(*) FROM users WHERE username = ?";\n    \n    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);\n    if (rc != SQLITE_OK) {\n        fprintf(stderr, "Database operation failed\\n");\n        sqlite3_close(db);\n        return false;\n    }\n    \n    /* Bind username parameter safely - prevents SQL injection */\n    rc = sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);\n    if (rc != SQLITE_OK) {\n        fprintf(stderr, "Database operation failed\\n");\n        sqlite3_finalize(stmt);\n        sqlite3_close(db);\n        return false;\n    }\n    \n    /* Execute query */\n    rc = sqlite3_step(stmt);\n    if (rc == SQLITE_ROW) {\n        int count = sqlite3_column_int(stmt, 0);\n        exists = (count > 0);\n    }\n    \n    /* Rules#8: Clean up resources in correct order */\n    sqlite3_finalize(stmt);\n    sqlite3_close(db);\n    \n    return exists;\n}\n\nint main(void) {\n    /* Test case 1: Valid username that exists */\n    printf("Test 1 (existing user): %d\\n", checkUsernameExists("alice"));\n    \n    /* Test case 2: Valid username that doesn't exist */
    printf("Test 2 (non-existing user): %d\\n", checkUsernameExists("nonexistent"));
    
    /* Test case 3: NULL username */
    printf("Test 3 (NULL): %d\\n", checkUsernameExists(NULL));
    
    /* Test case 4: Empty username */
    printf("Test 4 (empty): %d\\n", checkUsernameExists(""));
    
    /* Test case 5: Username with special characters */
    printf("Test 5 (special chars): %d\\n", checkUsernameExists("user@test"));
    
    return 0;
}
