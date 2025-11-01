
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

void setup_database(sqlite3* db) {
    char* errMsg = 0;
    const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                     "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                     "username TEXT UNIQUE NOT NULL,"
                     "password TEXT NOT NULL);";
    
    sqlite3_exec(db, sql, 0, 0, &errMsg);
    
    sqlite3_exec(db, "INSERT OR IGNORE INTO users (username, password) VALUES ('admin', 'admin123');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT OR IGNORE INTO users (username, password) VALUES ('user1', 'pass123');", 0, 0, &errMsg);
    sqlite3_exec(db, "INSERT OR IGNORE INTO users (username, password) VALUES ('testuser', 'testpass');", 0, 0, &errMsg);
}

const char* authenticate_user(const char* username, const char* password) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        return "error";
    }
    
    setup_database(db);
    
    const char* sql = "SELECT * FROM users WHERE username = ? AND password = ?;";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        sqlite3_close(db);
        return "error";
    }
    
    sqlite3_bind_text(stmt, 1, username, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password, -1, SQLITE_STATIC);
    
    int rc = sqlite3_step(stmt);
    const char* result;
    
    if (rc == SQLITE_ROW) {
        result = "dashboard";
    } else {
        result = "login";
    }
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    printf("Test Case 1:\\n");
    printf("Result: %s\\n", authenticate_user("admin", "admin123"));
    
    printf("\\nTest Case 2:\\n");
    printf("Result: %s\\n", authenticate_user("user1", "pass123"));
    
    printf("\\nTest Case 3:\\n");
    printf("Result: %s\\n", authenticate_user("wronguser", "wrongpass"));
    
    printf("\\nTest Case 4:\\n");
    printf("Result: %s\\n", authenticate_user("", ""));
    
    printf("\\nTest Case 5:\\n");
    printf("Result: %s\\n", authenticate_user("testuser", "testpass"));
    
    return 0;
}
