
#include <stdio.h>
#include <string.h>
#include <sqlite3.h>

void initialize_db(const char* db_path) {
    sqlite3* db;
    char* err_msg = NULL;
    
    if (sqlite3_open(db_path, &db) == SQLITE_OK) {
        const char* create_table = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password TEXT NOT NULL);";
        
        sqlite3_exec(db, create_table, NULL, NULL, &err_msg);
        
        const char* insert_user1 = 
            "INSERT OR IGNORE INTO users (username, password) VALUES ('admin', 'admin123');";
        const char* insert_user2 = 
            "INSERT OR IGNORE INTO users (username, password) VALUES ('user1', 'pass123');";
        
        sqlite3_exec(db, insert_user1, NULL, NULL, &err_msg);
        sqlite3_exec(db, insert_user2, NULL, NULL, &err_msg);
        
        sqlite3_close(db);
    }
}

const char* login(const char* username, const char* password, const char* db_path) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return "cant_login";
    }
    
    sqlite3* db;
    sqlite3_stmt* stmt;
    const char* result = "cant_login";
    
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        return "internal_server_error";
    }
    
    const char* query = "SELECT * FROM users WHERE username = ? AND password = ?";
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password, -1, SQLITE_TRANSIENT);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = "success";
        } else {
            result = "cant_login";
        }
        
        sqlite3_finalize(stmt);
    } else {
        result = "internal_server_error";
    }
    
    sqlite3_close(db);
    
    return result;
}

int main() {
    const char* db_path = "users.db";
    
    initialize_db(db_path);
    
    // Test cases
    printf("Test 1: %s\\n", login("admin", "admin123", db_path));
    printf("Test 2: %s\\n", login("user1", "pass123", db_path));
    printf("Test 3: %s\\n", login("invalid", "wrong", db_path));
    printf("Test 4: %s\\n", login("", "", db_path));
    printf("Test 5: %s\\n", login(NULL, NULL, db_path));
    
    return 0;
}
