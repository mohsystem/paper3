
#include <iostream>
#include <string>
#include <sqlite3.h>

class Lse36 {
public:
    static std::string login(const std::string& username, const std::string& password) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        int rc;
        
        // Open database
        rc = sqlite3_open("users.db", &db);
        if (rc) {
            std::string error = "error: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        // Prepare SQL query
        const char* sql = "SELECT password FROM users WHERE username = ?";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            std::string error = "error: " + std::string(sqlite3_errmsg(db));
            sqlite3_close(db);
            return error;
        }
        
        // Bind username parameter
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        // Execute query
        rc = sqlite3_step(stmt);
        
        std::string result;
        if (rc == SQLITE_ROW) {
            // Username exists, check password
            const unsigned char* storedPassword = sqlite3_column_text(stmt, 0);
            if (std::string(reinterpret_cast<const char*>(storedPassword)) == password) {
                result = "redirect:home";
            } else {
                result = "incorrect_password";
            }
        } else {
            result = "user_not_found";
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
    
    static void setupTestDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        sqlite3_open("users.db", &db);
        
        const char* createTable = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT)";
        sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg);
        
        sqlite3_exec(db, "DELETE FROM users", nullptr, nullptr, &errMsg);
        
        const char* insertUsers = 
            "INSERT OR REPLACE INTO users VALUES ('admin', 'admin123');"
            "INSERT OR REPLACE INTO users VALUES ('user1', 'pass123');"
            "INSERT OR REPLACE INTO users VALUES ('john_doe', 'johnspass');"
            "INSERT OR REPLACE INTO users VALUES ('testuser', 'testpass');"
            "INSERT OR REPLACE INTO users VALUES ('demo', 'demo123');";
        
        sqlite3_exec(db, insertUsers, nullptr, nullptr, &errMsg);
        sqlite3_close(db);
    }
};

int main() {
    // Setup test database
    Lse36::setupTestDatabase();
    
    // Test cases
    std::cout << "Test Case 1: " << Lse36::login("admin", "admin123") << std::endl;
    std::cout << "Test Case 2: " << Lse36::login("user1", "wrongpass") << std::endl;
    std::cout << "Test Case 3: " << Lse36::login("john_doe", "johnspass") << std::endl;
    std::cout << "Test Case 4: " << Lse36::login("nonexistent", "password") << std::endl;
    std::cout << "Test Case 5: " << Lse36::login("testuser", "testpass") << std::endl;
    
    return 0;
}
