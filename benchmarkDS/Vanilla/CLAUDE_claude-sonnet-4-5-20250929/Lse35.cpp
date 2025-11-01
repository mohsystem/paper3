
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <cstring>

class Lse35 {
private:
    static const char* DB_NAME;
    
public:
    static void initDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "username TEXT NOT NULL UNIQUE,"
                         "password TEXT NOT NULL)";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    static void addUser(const std::string& username, const std::string& password) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database" << std::endl;
            return;
        }
        
        const char* sql = "INSERT INTO users (username, password) VALUES (?, ?)";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
        
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
    
    static std::string login(const std::string& username, const std::string& password) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database" << std::endl;
            return "login.html";
        }
        
        const char* sql = "SELECT * FROM users WHERE username = ? AND password = ?";
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
        
        std::string result;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = "home.html";
        } else {
            result = "login.html";
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
};

const char* Lse35::DB_NAME = "users.db";

int main() {
    Lse35::initDatabase();
    
    // Clear existing data
    sqlite3* db;
    sqlite3_open(Lse35::DB_NAME, &db);
    sqlite3_exec(db, "DELETE FROM users", nullptr, nullptr, nullptr);
    sqlite3_close(db);
    
    // Add test users
    Lse35::addUser("admin", "admin123");
    Lse35::addUser("user1", "pass123");
    Lse35::addUser("john", "doe456");
    
    // Test cases
    std::cout << "Test Case 1: Valid credentials (admin/admin123)" << std::endl;
    std::cout << "Result: " << Lse35::login("admin", "admin123") << std::endl << std::endl;
    
    std::cout << "Test Case 2: Valid credentials (user1/pass123)" << std::endl;
    std::cout << "Result: " << Lse35::login("user1", "pass123") << std::endl << std::endl;
    
    std::cout << "Test Case 3: Invalid password" << std::endl;
    std::cout << "Result: " << Lse35::login("admin", "wrongpass") << std::endl << std::endl;
    
    std::cout << "Test Case 4: Non-existent user" << std::endl;
    std::cout << "Result: " << Lse35::login("nonuser", "password") << std::endl << std::endl;
    
    std::cout << "Test Case 5: Valid credentials (john/doe456)" << std::endl;
    std::cout << "Result: " << Lse35::login("john", "doe456") << std::endl;
    
    return 0;
}
