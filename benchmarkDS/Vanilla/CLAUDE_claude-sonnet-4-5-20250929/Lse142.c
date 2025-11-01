
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

char* unsubscribe(const char* email) {
    static char result[256];
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    
    rc = sqlite3_open("emaildb.db", &db);
    if (rc != SQLITE_OK) {
        snprintf(result, sizeof(result), "Cannot open database: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    // Check if email exists
    const char* checkQuery = "SELECT email FROM subscribers WHERE email = ?";
    rc = sqlite3_prepare_v2(db, checkQuery, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        snprintf(result, sizeof(result), "Failed to prepare statement: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        sqlite3_finalize(stmt);
        
        // Email exists, delete it
        const char* deleteQuery = "DELETE FROM subscribers WHERE email = ?";
        rc = sqlite3_prepare_v2(db, deleteQuery, -1, &stmt, NULL);
        
        if (rc != SQLITE_OK) {
            snprintf(result, sizeof(result), "Failed to prepare delete statement: %s", sqlite3_errmsg(db));
            sqlite3_close(db);
            return result;
        }
        
        sqlite3_bind_text(stmt, 1, email, -1, SQLITE_STATIC);
        rc = sqlite3_step(stmt);
        
        if (rc == SQLITE_DONE) {
            snprintf(result, sizeof(result), "Successfully unsubscribed: %s", email);
        } else {
            snprintf(result, sizeof(result), "Error occurred while unsubscribing");
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return result;
    } else {
        snprintf(result, sizeof(result), "Email not found in database: %s", email);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return result;
    }
}

int main() {
    // Test cases
    printf("Test Case 1:\\n%s\\n\\n", unsubscribe("user1@example.com"));
    printf("Test Case 2:\\n%s\\n\\n", unsubscribe("user2@example.com"));
    printf("Test Case 3:\\n%s\\n\\n", unsubscribe("notindb@example.com"));
    printf("Test Case 4:\\n%s\\n\\n", unsubscribe("user3@example.com"));
    printf("Test Case 5:\\n%s\\n\\n", unsubscribe("invalid@test.com"));
    
    return 0;
}
