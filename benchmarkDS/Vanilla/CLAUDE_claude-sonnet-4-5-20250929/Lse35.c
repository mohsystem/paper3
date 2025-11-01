
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

#define DB_NAME "users.db"

void initDatabase() {
    sqlite3* db;
    char* errMsg = NULL;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\\n", sqlite3_errmsg(db));
        return;
    }
    
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT NOT NULL UNIQUE,"
                     "password TEXT NOT NULL)";
    
    rc = sqlite3_exec(db, sql, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\\n", errMsg);
        sqlite3_free(errMsg);
    }
    
    sqlite3_close(db);
}

void addUser(const char* username, const char* password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Can't open database\\n");
        return;
    }
    
    const char* sql = "INSERT INTO users (username, password) VALUES (?, ?)";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_TRANSIENT);
    
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

const char* login(const char* username, const char* password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    int rc = sqlite3_open(DB_NAME, &db);
    if (rc) {
        fprintf(stderr, "Can't open database\\n");
        return "login.html";
    }
    
    const char* sql = "SELECT * FROM users WHERE username = ? AND password = ?";
    sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_TRANSIENT);
    
    const char* result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        result = "home.html";
    } else {
        result = "login.html";
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    initDatabase();
    
    // Clear existing data
    sqlite3* db;
    sqlite3_open(DB_NAME, &db);
    sqlite3_exec(db, "DELETE FROM users", NULL, NULL, NULL);
    sqlite3_close(db);
    
    // Add test users
    addUser("admin", "admin123");
    addUser("user1", "pass123");
    addUser("john", "doe456");
    
    // Test cases
    printf("Test Case 1: Valid credentials (admin/admin123)\\n");
    printf("Result: %s\\n\\n", login("admin", "admin123"));
    
    printf("Test Case 2: Valid credentials (user1/pass123)\\n");
    printf("Result: %s\\n\\n", login("user1", "pass123"));
    
    printf("Test Case 3: Invalid password\\n");
    printf("Result: %s\\n\\n", login("admin", "wrongpass"));
    
    printf("Test Case 4: Non-existent user\\n");
    printf("Result: %s\\n\\n", login("nonuser", "password"));
    
    printf("Test Case 5: Valid credentials (john/doe456)\\n");
    printf("Result: %s\\n", login("john", "doe456"));
    
    return 0;
}
