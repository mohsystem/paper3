
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cstring>

class Lse38 {
private:
    std::string db_path;
    
    void setupDatabase() {
        sqlite3* db;
        char* errMsg = 0;
        
        int rc = sqlite3_open(db_path.c_str(), &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "username TEXT UNIQUE NOT NULL,"
                         "password TEXT NOT NULL);";
        
        rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
        
        // Insert test users
        const char* insert1 = "INSERT OR IGNORE INTO users (username, password) VALUES ('admin', 'admin123');";
        const char* insert2 = "INSERT OR IGNORE INTO users (username, password) VALUES ('user1', 'pass123');";
        const char* insert3 = "INSERT OR IGNORE INTO users (username, password) VALUES ('testuser', 'testpass');";
        
        sqlite3_exec(db, insert1, 0, 0, &errMsg);
        sqlite3_exec(db, insert2, 0, 0, &errMsg);
        sqlite3_exec(db, insert3, 0, 0, &errMsg);
        
        sqlite3_close(db);
    }
    
public:
    Lse38(const std::string& path = "users.db") : db_path(path) {
        setupDatabase();
    }
    
    std::string authenticateUser(const std::string& username, const std::string& password) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(db_path.c_str(), &db);
        if (rc) {
            return "error";
        }
        
        const char* sql = "SELECT * FROM users WHERE username = ? AND password = ?;";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return "error";
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        std::string result;
        
        if (rc == SQLITE_ROW) {
            result = "dashboard";
        } else {
            result = "login";
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
};

int main() {
    Lse38 lse38;
    
    std::cout << "Test Case 1:" << std::endl;
    std::cout << "Result: " << lse38.authenticateUser("admin", "admin123") << std::endl;
    
    std::cout << "\\nTest Case 2:" << std::endl;
    std::cout << "Result: " << lse38.authenticateUser("user1", "pass123") << std::endl;
    
    std::cout << "\\nTest Case 3:" << std::endl;
    std::cout << "Result: " << lse38.authenticateUser("wronguser", "wrongpass") << std::endl;
    
    std::cout << "\\nTest Case 4:" << std::endl;
    std::cout << "Result: " << lse38.authenticateUser("", "") << std::endl;
    
    std::cout << "\\nTest Case 5:" << std::endl;
    std::cout << "Result: " << lse38.authenticateUser("testuser", "testpass") << std::endl;
    
    return 0;
}
