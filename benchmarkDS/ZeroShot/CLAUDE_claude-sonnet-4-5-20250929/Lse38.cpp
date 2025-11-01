
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

class Lse38 {
private:
    static const char* DB_NAME;
    
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    static void initDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* createTable = "CREATE TABLE IF NOT EXISTS users (username TEXT PRIMARY KEY, password TEXT)";
        sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg);
        sqlite3_exec(db, "DELETE FROM users", nullptr, nullptr, &errMsg);
        
        // Insert test users
        std::string testUsers[] = {"admin", "user1", "user2", "user3", "user4"};
        std::string testPasswords[] = {"admin123", "pass123", "password", "test123", "user123"};
        
        for(int i = 0; i < 5; i++) {
            std::string hashedPass = hashPassword(testPasswords[i]);
            sqlite3_stmt* stmt;
            
            const char* sql = "INSERT INTO users VALUES (?, ?)";
            sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
            sqlite3_bind_text(stmt, 1, testUsers[i].c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, hashedPass.c_str(), -1, SQLITE_STATIC);
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        
        sqlite3_close(db);
    }
    
    static bool login(const std::string& username, const std::string& password) {
        std::string hashedPassword = hashPassword(password);
        
        sqlite3* db;
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            return false;
        }
        
        sqlite3_stmt* stmt;
        const char* sql = "SELECT * FROM users WHERE username = ? AND password = ?";
        
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        bool found = (rc == SQLITE_ROW);
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return found;
    }
    
    static std::string authenticate(const std::string& username, const std::string& password) {
        if (login(username, password)) {
            return "REDIRECT: Dashboard - Welcome " + username + "!";
        } else {
            return "REDIRECT: Login Page - Invalid credentials";
        }
    }
};

const char* Lse38::DB_NAME = "users.db";

int main() {
    Lse38::initDatabase();
    
    std::cout << "=== Secure Login System Test Cases ===" << std::endl << std::endl;
    
    // Test case 1: Valid admin login
    std::cout << "Test 1 - Valid admin:" << std::endl;
    std::cout << Lse38::authenticate("admin", "admin123") << std::endl;
    
    // Test case 2: Valid user login
    std::cout << "\\nTest 2 - Valid user1:" << std::endl;
    std::cout << Lse38::authenticate("user1", "pass123") << std::endl;
    
    // Test case 3: Invalid password
    std::cout << "\\nTest 3 - Invalid password:" << std::endl;
    std::cout << Lse38::authenticate("admin", "wrongpass") << std::endl;
    
    // Test case 4: Non-existent user
    std::cout << "\\nTest 4 - Non-existent user:" << std::endl;
    std::cout << Lse38::authenticate("hacker", "hack123") << std::endl;
    
    // Test case 5: SQL injection attempt
    std::cout << "\\nTest 5 - SQL injection attempt:" << std::endl;
    std::cout << Lse38::authenticate("admin' OR '1'='1", "anything") << std::endl;
    
    return 0;
}
