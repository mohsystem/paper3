
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>
#include <sqlite3.h>
#include <ctype.h>

#define MAX_EMAIL_LEN 256
#define MAX_MESSAGE_LEN 512

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
        str[newLen] = '\\0';
    } else {
        str[0] = '\\0';
    }
}

// Validate email format using regex
int isValidEmail(const char* email) {
    regex_t regex;
    int reti;
    
    const char* pattern = "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$";
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    
    if (reti) {
        return 0;
    }
    
    reti = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return (reti == 0);
}

// Secure unsubscribe function
char* unsubscribe(const char* emailInput, const char* dbPath, char* result) {
    sqlite3* db = NULL;
    sqlite3_stmt* checkStmt = NULL;
    sqlite3_stmt* deleteStmt = NULL;
    char email[MAX_EMAIL_LEN];
    
    // Initialize result
    if (result == NULL) {
        return NULL;
    }
    
    // Input validation
    if (emailInput == NULL || strlen(emailInput) == 0) {
        strcpy(result, "Error: Email address is required");
        return result;
    }
    
    strncpy(email, emailInput, MAX_EMAIL_LEN - 1);
    email[MAX_EMAIL_LEN - 1] = '\\0';
    trim(email);
    
    // Validate email format
    if (!isValidEmail(email)) {
        strcpy(result, "Error: Invalid email format");
        return result;
    }
    
    // Secure database connection
    if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
        strcpy(result, "Error: Database connection failed");
        if (db) sqlite3_close(db);
        return result;
    }
    
    // Check if email exists using parameterized query
    const char* checkQuery = "SELECT email FROM subscribers WHERE email = ?";
    if (sqlite3_prepare_v2(db, checkQuery, -1, &checkStmt, NULL) != SQLITE_OK) {
        strcpy(result, "Error: Database operation failed");
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(checkStmt, 1, email, -1, SQLITE_STATIC);
    int queryResult = sqlite3_step(checkStmt);
    
    if (queryResult == SQLITE_ROW) {
        // Email exists, remove it using parameterized query
        const char* deleteQuery = "DELETE FROM subscribers WHERE email = ?";
        if (sqlite3_prepare_v2(db, deleteQuery, -1, &deleteStmt, NULL) != SQLITE_OK) {
            strcpy(result, "Error: Database operation failed");
            sqlite3_finalize(checkStmt);
            sqlite3_close(db);
            return result;
        }
        
        sqlite3_bind_text(deleteStmt, 1, email, -1, SQLITE_STATIC);
        
        if (sqlite3_step(deleteStmt) == SQLITE_DONE) {
            strcpy(result, "Success: Email address has been unsubscribed");
        } else {
            strcpy(result, "Error: Failed to unsubscribe");
        }
        
        sqlite3_finalize(deleteStmt);
    } else {
        strcpy(result, "Info: Email address is not in the database");
    }
    
    sqlite3_finalize(checkStmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    char result[MAX_MESSAGE_LEN];
    
    // Test cases
    printf("Test 1: %s\\n", unsubscribe("user@example.com", "newsletter.db", result));
    printf("Test 2: %s\\n", unsubscribe("test@domain.org", "newsletter.db", result));
    printf("Test 3: %s\\n", unsubscribe("invalid-email", "newsletter.db", result));
    printf("Test 4: %s\\n", unsubscribe("", "newsletter.db", result));
    printf("Test 5: %s\\n", unsubscribe("nonexistent@email.com", "newsletter.db", result));
    
    return 0;
}
