
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

char* login(const char* username, const char* password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    int rc;
    static char result[100];
    
    // Open database
    rc = sqlite3_open("users.db", &db);
    if (rc) {
        snprintf(result, sizeof(result), "error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    // Prepare SQL query
    const char* sql = "SELECT password FROM users WHERE username = ?";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    
    if (rc != SQLITE_OK) {
        snprintf(result, sizeof(result), "error: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return result;
    }
    
    // Bind username parameter
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    
    // Execute query
    rc = sqlite3_step(stmt);
    
    if (rc == SQLITE_ROW) {
        // Username exists, check password
        const unsigned char* storedPassword = sqlite3_column_text(stmt, 0);
        if (strcmp((const char*)storedPassword, password) == 0) {
            strcpy(result, "redirect:home");
        } else {
            strcpy(result, "incorrect_password");
        }
    } else {
        strcpy(result, "user_not_found");
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

void setupTestDatabase() {
    sqlite3* db;
    char* errMsg = NULL;
    
    sqlite3_open("users.db", &db);
    
    const char* createTable = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT)";
    sqlite3_exec(db, createTable, NULL, NULL, &errMsg);
    
    sqlite3_exec(db, "DELETE FROM users", NULL, NULL, &errMsg);
    
    const char* insertUsers = 
        "INSERT OR REPLACE INTO users VALUES ('admin', 'admin123');"
        "INSERT OR REPLACE INTO users VALUES ('user1', 'pass123');"
        "INSERT OR REPLACE INTO users VALUES ('john_doe', 'johnspass');"
        "INSERT OR REPLACE INTO users VALUES ('testuser', 'testpass');"
        "INSERT OR REPLACE INTO users VALUES ('demo', 'demo123');";
    
    sqlite3_exec(db, insertUsers, NULL, NULL, &errMsg);
    sqlite3_close(db);
}

int main() {
    // Setup test database
    setupTestDatabase();
    
    // Test cases
    printf("Test Case 1: %s\\n", login("admin", "admin123"));
    printf("Test Case 2: %s\\n", login("user1", "wrongpass"));
    printf("Test Case 3: %s\\n", login("john_doe", "johnspass"));
    printf("Test Case 4: %s\\n", login("nonexistent", "password"));
    printf("Test Case 5: %s\\n", login("testuser", "testpass"));
    
    return 0;
}
