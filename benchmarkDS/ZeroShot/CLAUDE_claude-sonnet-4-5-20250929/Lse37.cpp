
#include <iostream>
#include <string>
#include <sqlite3.h>

class Lse37 {
private:
    std::string dbPath;
    
    void initializeDb() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        if (sqlite3_open(dbPath.c_str(), &db) == SQLITE_OK) {
            const char* createTable = 
                "CREATE TABLE IF NOT EXISTS users ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "username TEXT UNIQUE NOT NULL,"
                "password TEXT NOT NULL);";
            
            sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg);
            
            const char* insertUser1 = 
                "INSERT OR IGNORE INTO users (username, password) VALUES ('admin', 'admin123');";
            const char* insertUser2 = 
                "INSERT OR IGNORE INTO users (username, password) VALUES ('user1', 'pass123');";
            
            sqlite3_exec(db, insertUser1, nullptr, nullptr, &errMsg);
            sqlite3_exec(db, insertUser2, nullptr, nullptr, &errMsg);
            
            sqlite3_close(db);
        }
    }

public:
    Lse37(const std::string& path = "users.db") : dbPath(path) {
        initializeDb();
    }
    
    std::string login(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "cant_login";
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        std::string result = "cant_login";
        
        try {
            if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
                return "internal_server_error";
            }
            
            const char* query = "SELECT * FROM users WHERE username = ? AND password = ?";
            
            if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
                
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
            
        } catch (...) {
            result = "internal_server_error";
        }
        
        return result;
    }
};

int main() {
    Lse37 lse;
    
    // Test cases
    std::cout << "Test 1: " << lse.login("admin", "admin123") << std::endl;
    std::cout << "Test 2: " << lse.login("user1", "pass123") << std::endl;
    std::cout << "Test 3: " << lse.login("invalid", "wrong") << std::endl;
    std::cout << "Test 4: " << lse.login("", "") << std::endl;
    std::cout << "Test 5: " << lse.login("test", "") << std::endl;
    
    return 0;
}
