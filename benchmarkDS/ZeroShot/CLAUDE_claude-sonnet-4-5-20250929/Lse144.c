
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <sqlite3.h>

#define EMAIL_REGEX "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$"
#define DB_PATH "emaildb.db"
#define MAX_EMAIL_LENGTH 256

/**
 * Validates email format using regex
 */
int isValidEmail(const char* email) {
    if (email == NULL || strlen(email) == 0) {
        return 0;
    }
    
    regex_t regex;
    int result;
    
    // Compile regex
    result = regcomp(&regex, EMAIL_REGEX, REG_EXTENDED);
    if (result != 0) {
        return 0;
    }
    
    // Execute regex
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return (result == 0) ? 1 : 0;
}

/**
 * Simulates calling an unsubscribe script
 * Returns 0 on success, non-zero on failure
 */
int callUnsubscribeScript(const char* email) {
    // In a real implementation, this would call an actual script
    // For security, validate and sanitize input before passing to external script
    
    // Simulate script execution
    // Return 0 for success
    return 0;
}

/**
 * Checks if email exists in database and unsubscribes if found
 * Returns 1 if successfully unsubscribed, 0 otherwise
 */
int unsubscribeEmail(const char* email) {
    // Validate email format first
    if (!isValidEmail(email)) {
        return 0;
    }
    
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc;
    int result = 0;
    
    // Open database connection
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    // Check if email exists using parameterized query (prevents SQL injection)
    const char* checkQuery = "SELECT email FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, checkQuery, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Failed to prepare statement: %s\\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 0;
    }
    
    // Bind email parameter
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);
    
    // Execute query
    rc = sqlite3_step(stmt);
    
    // If email doesn't exist, return 0
    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 0;
    }
    
    // Email exists, call unsubscribe script
    int unsubscribeResult = callUnsubscribeScript(email);
    
    // If script returns 0 (success), return 1, otherwise return 0
    result = (unsubscribeResult == 0) ? 1 : 0;
    
    // Clean up resources
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    // Test cases
    printf("Test Case 1 (valid existing email): %d\\n", unsubscribeEmail("user1@example.com"));
    printf("Test Case 2 (valid non-existing email): %d\\n", unsubscribeEmail("nonexistent@example.com"));
    printf("Test Case 3 (invalid email format): %d\\n", unsubscribeEmail("invalid-email"));
    printf("Test Case 4 (null email): %d\\n", unsubscribeEmail(NULL));
    printf("Test Case 5 (empty email): %d\\n", unsubscribeEmail(""));
    
    return 0;
}
