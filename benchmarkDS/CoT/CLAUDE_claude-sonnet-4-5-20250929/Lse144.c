
#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <sqlite3.h>

#define EMAIL_REGEX "^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$"

int unsubscribeEmail(const char* email) {
    // Simulated unsubscribe script call
    return 0; // Success
}

int isValidEmail(const char* email) {
    regex_t regex;
    int ret;
    
    ret = regcomp(&regex, EMAIL_REGEX, REG_EXTENDED);
    if (ret != 0) {
        return 0;
    }
    
    ret = regexec(&regex, email, 0, NULL, 0);
    regfree(&regex);
    
    return (ret == 0) ? 1 : 0;
}

int processUnsubscribe(const char* email, sqlite3* db) {
    if (email == NULL || strlen(email) == 0 || db == NULL) {
        return 0;
    }
    
    if (!isValidEmail(email)) {
        return 0;
    }
    
    sqlite3_stmt* stmt = NULL;
    const char* query = "SELECT COUNT(*) FROM users WHERE email = ?";
    
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return 0;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_TRANSIENT);
    
    int result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        if (count > 0) {
            int scriptResult = unsubscribeEmail(email);
            result = (scriptResult == 0) ? 1 : 0;
        }
    }
    
    sqlite3_finalize(stmt);
    return result;
}

int main() {
    sqlite3* db = NULL;
    sqlite3_open(":memory:", &db);
    
    sqlite3_exec(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, email TEXT)", 
                 NULL, NULL, NULL);
    sqlite3_exec(db, "INSERT INTO users (email) VALUES ('test@example.com')", 
                 NULL, NULL, NULL);
    
    printf("Test 1: %d\\n", processUnsubscribe("test@example.com", db));
    printf("Test 2: %d\\n", processUnsubscribe("nonexistent@example.com", db));
    printf("Test 3: %d\\n", processUnsubscribe("invalid-email", db));
    printf("Test 4: %d\\n", processUnsubscribe(NULL, db));
    printf("Test 5: %d\\n", processUnsubscribe("", db));
    
    sqlite3_close(db);
    return 0;
}
