
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <regex.h>
#include <ctype.h>

#define DB_PATH "mailing_list.db"
#define MAX_EMAIL_LEN 256
#define EMAIL_REGEX "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$"

void trim(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\\0';
}

int validate_email(const char* email) {
    regex_t regex;
    int result;
    
    if (regcomp(&regex, EMAIL_REGEX, REG_EXTENDED) != 0) {
        return 0;
    }
    
    result = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0;
}

char* unsubscribe(const char* email_param) {
    static char result[256];
    char email[MAX_EMAIL_LEN];
    sqlite3* db = NULL;
    sqlite3_stmt* stmt = NULL;
    int rc;
    
    // Input validation
    if (email_param == NULL || strlen(email_param) == 0) {
        strcpy(result, "Invalid email parameter");
        return result;
    }
    
    strncpy(email, email_param, MAX_EMAIL_LEN - 1);
    email[MAX_EMAIL_LEN - 1] = '\\0';
    trim(email);
    
    // Validate email format
    if (!validate_email(email)) {
        strcpy(result, "Invalid email format");
        return result;
    }
    
    // Open database
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        snprintf(result, sizeof(result), "Database error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    // Check if email exists
    const char* check_query = "SELECT email FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, check_query, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        snprintf(result, sizeof(result), "Database error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        // Email exists, delete the user
        sqlite3_finalize(stmt);
        
        const char* delete_query = "DELETE FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, delete_query, -1, &stmt, NULL);
        
        if (rc != SQLITE_OK) {
            snprintf(result, sizeof(result), "Database error: %s", sqlite3_errmsg(db));
            sqlite3_close(db);
            return result;
        }
        
        sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (rc == SQLITE_DONE) {
            snprintf(result, sizeof(result), "Successfully unsubscribed: %s", email);
        } else {
            strcpy(result, "Failed to unsubscribe");
        }
    } else {
        // Email not found
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        strcpy(result, "User is not subscribed");
    }
    
    return result;
}

void init_test_database() {
    sqlite3* db = NULL;
    char* err_msg = NULL;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* create_table = "CREATE TABLE IF NOT EXISTS subscribers (id INTEGER PRIMARY KEY AUTOINCREMENT, email TEXT UNIQUE NOT NULL)";
    sqlite3_exec(db, create_table, NULL, NULL, &err_msg);
    
    sqlite3_exec(db, "DELETE FROM subscribers", NULL, NULL, &err_msg);
    sqlite3_exec(db, "INSERT INTO subscribers (email) VALUES ('test1@example.com')", NULL, NULL, &err_msg);
    sqlite3_exec(db, "INSERT INTO subscribers (email) VALUES ('test2@example.com')", NULL, NULL, &err_msg);
    sqlite3_exec(db, "INSERT INTO subscribers (email) VALUES ('user@domain.com')", NULL, NULL, &err_msg);
    
    sqlite3_close(db);
}

int main() {
    // Initialize test database
    init_test_database();
    
    // Test case 1: Valid unsubscribe
    printf("Test 1: %s\\n", unsubscribe("test1@example.com"));
    
    // Test case 2: User not subscribed
    printf("Test 2: %s\\n", unsubscribe("notfound@example.com"));
    
    // Test case 3: Another valid unsubscribe
    printf("Test 3: %s\\n", unsubscribe("test2@example.com"));
    
    // Test case 4: Invalid email format
    printf("Test 4: %s\\n", unsubscribe("invalid-email"));
    
    // Test case 5: Empty email
    printf("Test 5: %s\\n", unsubscribe(""));
    
    return 0;
}
