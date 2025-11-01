
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <ctype.h>
#include <stdbool.h>

// Trim whitespace from string
void trim(char* str) {
    if (str == NULL) return;
    
    int len = strlen(str);
    int start = 0, end = len - 1;
    
    while (start < len && isspace(str[start])) start++;
    while (end >= start && isspace(str[end])) end--;
    
    int newLen = end - start + 1;
    if (newLen > 0) {
        memmove(str, str + start, newLen);
    }
    str[newLen] = '\\0';
}

// Basic email validation
bool isValidEmail(const char* email) {
    if (email == NULL || strlen(email) == 0) return false;
    
    int atCount = 0;
    int dotAfterAt = 0;
    bool foundAt = false;
    
    for (int i = 0; email[i] != '\\0'; i++) {
        if (email[i] == '@') {
            atCount++;
            foundAt = true;
            if (i == 0) return false; // @ at start
        } else if (foundAt && email[i] == '.') {
            dotAfterAt++;
        }
    }
    
    return (atCount == 1 && dotAfterAt > 0);
}

char* unsubscribe(const char* email, sqlite3* db, char* result, size_t resultSize) {
    if (email == NULL || strlen(email) == 0) {
        snprintf(result, resultSize, "Error: Email address cannot be empty.");
        return result;
    }
    
    char trimmedEmail[256];
    strncpy(trimmedEmail, email, sizeof(trimmedEmail) - 1);
    trimmedEmail[sizeof(trimmedEmail) - 1] = '\\0';
    trim(trimmedEmail);
    
    if (!isValidEmail(trimmedEmail)) {
        snprintf(result, resultSize, "Error: Invalid email address format.");
        return result;
    }
    
    sqlite3_stmt* stmt;
    int rc;
    
    // Check if email exists
    const char* checkQuery = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, checkQuery, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        snprintf(result, resultSize, "Error: Database operation failed.");
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, trimmedEmail, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    
    int count = 0;
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    
    if (count > 0) {
        // Email exists, delete it
        const char* deleteQuery = "DELETE FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, deleteQuery, -1, &stmt, NULL);
        
        if (rc != SQLITE_OK) {
            snprintf(result, resultSize, "Error: Database operation failed.");
            return result;
        }
        
        sqlite3_bind_text(stmt, 1, trimmedEmail, -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc == SQLITE_DONE) {
            snprintf(result, resultSize, "Success: Email address has been unsubscribed.");
            return result;
        }
    } else {
        snprintf(result, resultSize, "Info: Email address not found in database.");
        return result;
    }
    
    snprintf(result, resultSize, "Error: Unsubscribe operation failed.");
    return result;
}

int main() {
    sqlite3* db;
    char result[256];
    
    // Setup in-memory database
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database\\n");
        return 1;
    }
    
    // Create table and insert test data
    char* errMsg;
    sqlite3_exec(db, "CREATE TABLE subscribers (email TEXT PRIMARY KEY)", NULL, NULL, &errMsg);
    sqlite3_exec(db, "INSERT INTO subscribers VALUES ('test1@example.com')", NULL, NULL, &errMsg);
    sqlite3_exec(db, "INSERT INTO subscribers VALUES ('test2@example.com')", NULL, NULL, &errMsg);
    sqlite3_exec(db, "INSERT INTO subscribers VALUES ('user@domain.com')", NULL, NULL, &errMsg);
    
    // Test Case 1: Valid email that exists
    printf("Test 1: %s\\n", unsubscribe("test1@example.com", db, result, sizeof(result)));
    
    // Test Case 2: Valid email that doesn't exist
    printf("Test 2: %s\\n", unsubscribe("nonexistent@example.com", db, result, sizeof(result)));
    
    // Test Case 3: Invalid email format
    printf("Test 3: %s\\n", unsubscribe("invalid-email", db, result, sizeof(result)));
    
    // Test Case 4: Empty email
    printf("Test 4: %s\\n", unsubscribe("", db, result, sizeof(result)));
    
    // Test Case 5: Valid email with whitespace
    printf("Test 5: %s\\n", unsubscribe("  test2@example.com  ", db, result, sizeof(result)));
    
    sqlite3_close(db);
    return 0;
}
