
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <regex.h>

/* Rules#2: Maximum email length per RFC 5321 */
#define MAX_EMAIL_LENGTH 254
#define EMAIL_PATTERN "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$"

/**
 * Validate email format using regex
 * Rules#2: Format validation with allow list pattern
 */
int validate_email_format(const char* email) {
    regex_t regex;
    int ret;
    
    /* Rules#36: Check for NULL pointer */
    if (email == NULL) {
        return 0;
    }
    
    ret = regcomp(&regex, EMAIL_PATTERN, REG_EXTENDED | REG_NOSUB);
    if (ret != 0) {
        return 0;
    }
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return (ret == 0) ? 1 : 0;
}

/**
 * Unsubscribe function with security controls
 * Rules#2: Validates input length and format
 * Rules#32: Uses parameterized queries to prevent SQL injection
 * Rules#11: Returns generic error messages
 * Rules#38: Returns allocated string that caller must free
 */
char* unsubscribe(const char* email) {
    char* result = NULL;
    sqlite3* db = NULL;
    sqlite3_stmt* check_stmt = NULL;
    sqlite3_stmt* delete_stmt = NULL;
    const char* db_path = NULL;
    int rc;
    int count = 0;
    
    /* Rules#36: Validate input is not NULL */
    if (email == NULL) {
        result = (char*)malloc(25);
        if (result != NULL) {
            snprintf(result, 25, "Invalid email address");
        }
        return result;
    }
    
    /* Rules#2: Validate length */
    size_t email_len = strnlen(email, MAX_EMAIL_LENGTH + 1);
    if (email_len > MAX_EMAIL_LENGTH) {
        result = (char*)malloc(25);
        if (result != NULL) {
            snprintf(result, 25, "Invalid email address");
        }
        return result;
    }
    
    /* Rules#2: Validate format */
    if (!validate_email_format(email)) {
        result = (char*)malloc(25);
        if (result != NULL) {
            snprintf(result, 25, "Invalid email address");
        }
        return result;
    }
    
    /* Rules#3: Load database path from environment */
    db_path = getenv("DB_PATH");
    if (db_path == NULL) {
        db_path = "subscribers.db";
    }
    
    /* Open database connection */
    rc = sqlite3_open(db_path, &db);
    if (rc != SQLITE_OK) {
        /* Rules#11: Log detailed error, return generic message */
        fprintf(stderr, "Database error: %s\\n", sqlite3_errmsg(db));
        result = (char*)malloc(25);
        if (result != NULL) {
            snprintf(result, 25, "Service error occurred");
        }
        goto cleanup;
    }
    
    /* Begin transaction */
    sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
    
    /* Rules#32: Use parameterized query to prevent SQL injection */
    const char* check_query = "SELECT COUNT(*) FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, check_query, -1, &check_stmt, NULL);
    
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare error: %s\\n", sqlite3_errmsg(db));
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        result = (char*)malloc(25);
        if (result != NULL) {
            snprintf(result, 25, "Service error occurred");
        }
        goto cleanup;
    }
    
    /* Bind email parameter safely */
    sqlite3_bind_text(check_stmt, 1, email, -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(check_stmt);
    if (rc == SQLITE_ROW) {
        count = sqlite3_column_int(check_stmt, 0);
        
        if (count > 0) {
            /* Email exists, remove it */
            /* Rules#32: Parameterized query prevents SQL injection */
            const char* delete_query = "DELETE FROM subscribers WHERE email = ?";
            rc = sqlite3_prepare_v2(db, delete_query, -1, &delete_stmt, NULL);
            
            if (rc != SQLITE_OK) {
                fprintf(stderr, "Delete prepare error: %s\\n", sqlite3_errmsg(db));
                sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
                result = (char*)malloc(25);
                if (result != NULL) {
                    snprintf(result, 25, "Service error occurred");
                }
                goto cleanup;
            }
            
            sqlite3_bind_text(delete_stmt, 1, email, -1, SQLITE_TRANSIENT);
            
            rc = sqlite3_step(delete_stmt);
            if (rc == SQLITE_DONE) {
                sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
                /* Rules#10: Do not log user email */
                result = (char*)malloc(27);
                if (result != NULL) {
                    snprintf(result, 27, "Successfully unsubscribed");
                }
            } else {
                sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
                result = (char*)malloc(21);
                if (result != NULL) {
                    snprintf(result, 21, "Unsubscribe failed");
                }
            }
        } else {
            /* Email not in database */
            sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
            result = (char*)malloc(40);
            if (result != NULL) {
                snprintf(result, 40, "Email address not found in database");
            }
        }
    } else {
        sqlite3_exec(db, "ROLLBACK", NULL, NULL, NULL);
        result = (char*)malloc(25);
        if (result != NULL) {
            snprintf(result, 25, "Service error occurred");
        }
    }
    
cleanup:
    /* Rules#38: Proper resource cleanup */
    if (check_stmt) sqlite3_finalize(check_stmt);
    if (delete_stmt) sqlite3_finalize(delete_stmt);
    if (db) sqlite3_close(db);
    
    return result;
}

int main(void) {
    char* result = NULL;
    
    /* Test cases */
    printf("Test 1 - Valid email: ");
    result = unsubscribe("user@example.com");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("Test 2 - Invalid format: ");
    result = unsubscribe("invalid-email");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("Test 3 - NULL input: ");
    result = unsubscribe(NULL);
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("Test 4 - Too long: ");
    char long_email[300];
    memset(long_email, 'a', 299);
    long_email[299] = '\\0';
    result = unsubscribe(long_email);
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    printf("Test 5 - SQL injection attempt: ");
    result = unsubscribe("user@example.com' OR '1'='1");
    if (result != NULL) {
        printf("%s\\n", result);
        free(result);
    }
    
    return 0;
}
