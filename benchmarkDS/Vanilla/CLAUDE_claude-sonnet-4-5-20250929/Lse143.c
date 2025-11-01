
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

#define DB_PATH "mailing_list.db"
#define MAX_RESULT_LEN 256

char* unsubscribe(const char* email) {
    char* result = (char*)malloc(MAX_RESULT_LEN * sizeof(char));
    
    if (email == NULL || strlen(email) == 0) {
        strcpy(result, "Invalid email address");
        return result;
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    
    rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        snprintf(result, MAX_RESULT_LEN, "Cannot open database: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    const char* selectQuery = "SELECT email FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, selectQuery, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        snprintf(result, MAX_RESULT_LEN, "SQL error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        
        const char* deleteQuery = "DELETE FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, deleteQuery, -1, &stmt, NULL);
        
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
            rc = sqlite3_step(stmt);
            
            if (rc == SQLITE_DONE) {
                snprintf(result, MAX_RESULT_LEN, "Successfully unsubscribed: %s", email);
            } else {
                snprintf(result, MAX_RESULT_LEN, "Error deleting user: %s", sqlite3_errmsg(db));
            }
        } else {
            snprintf(result, MAX_RESULT_LEN, "SQL error: %s", sqlite3_errmsg(db));
        }
    } else {
        snprintf(result, MAX_RESULT_LEN, "User is not subscribed: %s", email);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

void initializeDatabase() {
    sqlite3* db;
    char* errMsg = NULL;
    
    int rc = sqlite3_open(DB_PATH, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS subscribers ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "email TEXT UNIQUE NOT NULL)";
    
    rc = sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
    
    const char* testEmails[] = {
        "user1@example.com",
        "user2@example.com",
        "user3@example.com"
    };
    
    for (int i = 0; i < 3; i++) {
        char insertSQL[256];
        snprintf(insertSQL, sizeof(insertSQL), 
                "INSERT OR IGNORE INTO subscribers (email) VALUES ('%s')", testEmails[i]);
        sqlite3_exec(db, insertSQL, NULL, NULL, &errMsg);
    }
    
    sqlite3_close(db);
}

int main() {
    initializeDatabase();
    
    char* result;
    
    printf("Test 1: ");
    result = unsubscribe("user1@example.com");
    printf("%s\\n", result);
    free(result);
    
    printf("Test 2: ");
    result = unsubscribe("user1@example.com");
    printf("%s\\n", result);
    free(result);
    
    printf("Test 3: ");
    result = unsubscribe("user2@example.com");
    printf("%s\\n", result);
    free(result);
    
    printf("Test 4: ");
    result = unsubscribe("nonexistent@example.com");
    printf("%s\\n", result);
    free(result);
    
    printf("Test 5: ");
    result = unsubscribe("");
    printf("%s\\n", result);
    free(result);
    
    return 0;
}
