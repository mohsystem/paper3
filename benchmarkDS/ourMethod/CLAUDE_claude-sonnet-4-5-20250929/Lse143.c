
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sqlite3.h>
#include <regex.h>

/* Maximum email length to prevent DOS */
#define MAX_EMAIL_LENGTH 254
#define MAX_RESPONSE_LENGTH 256

/* Email validation pattern - simplified for C */
#define EMAIL_PATTERN "^[a-zA-Z0-9_+&*-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,7}$"

/**
 * Trims whitespace from both ends of string in place
 */
void trim(char* str) {
    if (str == NULL) return;
    
    char* start = str;
    while (isspace((unsigned char)*start)) start++;
    
    if (*start == '\\0') {
        str[0] = '\\0';
        return;
    }
    
    char* end = str + strlen(str) - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    size_t len = (end - start + 1);
    memmove(str, start, len);
    str[len] = '\\0';
}

/**
 * Converts string to lowercase in place
 */
void toLowerStr(char* str) {
    if (str == NULL) return;
    
    for (size_t i = 0; str[i]; i++) {
        str[i] = tolower((unsigned char)str[i]);
    }
}

/**
 * Validates email format using regex
 */
int isValidEmail(const char* email) {
    if (email == NULL || strlen(email) == 0 || strlen(email) > MAX_EMAIL_LENGTH) {
        return 0;
    }
    
    regex_t regex;
    int ret = regcomp(&regex, EMAIL_PATTERN, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        return 0;
    }
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return (ret == 0) ? 1 : 0;
}

/**
 * Initializes database with subscribers table
 */
void initializeDatabase(const char* dbPath) {
    sqlite3* db = NULL;
    char* errMsg = NULL;
    
    /* Open database connection */
    int rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database initialization failed\\n");
        if (db) sqlite3_close(db);
        return;
    }
    
    /* Create table if not exists */
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS subscribers ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "email TEXT UNIQUE NOT NULL)";
    
    rc = sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database initialization failed\\n");
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

/**
 * Handles unsubscribe request
 */
char* handleUnsubscribe(const char* emailInput) {
    /* Allocate response buffer */
    char* response = (char*)malloc(MAX_RESPONSE_LENGTH);
    if (response == NULL) {
        return NULL;
    }
    memset(response, 0, MAX_RESPONSE_LENGTH);
    
    /* Input validation - check for NULL/empty */
    if (emailInput == NULL || strlen(emailInput) == 0) {
        snprintf(response, MAX_RESPONSE_LENGTH, "Invalid request: email parameter is required");
        return response;
    }
    
    /* Create mutable copy for processing */
    size_t emailLen = strlen(emailInput);
    if (emailLen > MAX_EMAIL_LENGTH) {
        snprintf(response, MAX_RESPONSE_LENGTH, "Invalid email format");
        return response;
    }
    
    char* email = (char*)malloc(emailLen + 1);
    if (email == NULL) {
        snprintf(response, MAX_RESPONSE_LENGTH, "An error occurred processing your request");
        return response;
    }
    
    strncpy(email, emailInput, emailLen);
    email[emailLen] = '\\0';
    
    /* Trim and normalize input */
    trim(email);
    toLowerStr(email);
    
    /* Validate email format */
    if (!isValidEmail(email)) {
        snprintf(response, MAX_RESPONSE_LENGTH, "Invalid email format");
        free(email);
        return response;
    }
    
    /* Get database path from environment or use default */
    const char* dbPath = getenv("DB_PATH");
    if (dbPath == NULL) {
        dbPath = "subscribers.db";
    }
    
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    
    /* Open database connection */
    int rc = sqlite3_open(dbPath, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database error occurred\\n");
        snprintf(response, MAX_RESPONSE_LENGTH, "An error occurred processing your request");
        if (db) sqlite3_close(db);
        free(email);
        return response;
    }
    
    /* Check if email exists - using parameterized query to prevent SQL injection */
    const char* checkSQL = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, checkSQL, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Database error occurred\\n");
        snprintf(response, MAX_RESPONSE_LENGTH, "An error occurred processing your request");
        sqlite3_close(db);
        free(email);
        return response;
    }
    
    /* Bind email parameter */
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        stmt = NULL;
        
        if (count > 0) {
            /* Email exists, proceed with deletion using parameterized query */
            const char* deleteSQL = "DELETE FROM subscribers WHERE email = ?";
            rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, NULL);
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Database error occurred\\n");
                snprintf(response, MAX_RESPONSE_LENGTH, "An error occurred processing your request");
                sqlite3_close(db);
                free(email);
                return response;
            }
            
            sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);
            rc = sqlite3_step(stmt);
            
            if (rc == SQLITE_DONE && sqlite3_changes(db) > 0) {
                snprintf(response, MAX_RESPONSE_LENGTH, "Successfully unsubscribed");
            } else {
                snprintf(response, MAX_RESPONSE_LENGTH, "Unsubscribe failed");
            }
        } else {
            /* Email not found in database */
            snprintf(response, MAX_RESPONSE_LENGTH, "Email address is not subscribed");
        }
    } else {
        fprintf(stderr, "Database error occurred\\n");
        snprintf(response, MAX_RESPONSE_LENGTH, "An error occurred processing your request");
    }
    
    /* Cleanup */
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    free(email);
    
    return response;
}

int main() {
    /* Initialize database */
    const char* dbPath = getenv("DB_PATH");
    if (dbPath == NULL) {
        dbPath = "subscribers.db";
    }
    initializeDatabase(dbPath);
    
    /* Test cases */
    char* result = NULL;
    
    printf("Test Case 1 - Valid email unsubscribe:\\n");
    result = handleUnsubscribe("test@example.com");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("\\nTest Case 2 - Email not in database:\\n");
    result = handleUnsubscribe("notfound@example.com");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("\\nTest Case 3 - Invalid email format:\\n");
    result = handleUnsubscribe("invalid-email");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("\\nTest Case 4 - NULL email:\\n");
    result = handleUnsubscribe(NULL);
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("\\nTest Case 5 - Empty email:\\n");
    result = handleUnsubscribe("");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    return 0;
}
