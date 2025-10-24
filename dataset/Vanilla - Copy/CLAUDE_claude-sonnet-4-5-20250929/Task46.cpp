
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

class Task46 {
private:
    std::string dbName;
    
    std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
        }
        return ss.str();
    }
    
public:
    Task46(const std::string& dbName = "users.db") : dbName(dbName) {
        initializeDatabase();
    }
    
    void initializeDatabase() {
        sqlite3* db;
        char* errMsg = 0;
        
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "name TEXT NOT NULL,"
                         "email TEXT UNIQUE NOT NULL,"
                         "password TEXT NOT NULL);";
        
        rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    bool registerUser(const std::string& name, const std::string& email, const std::string& password) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        std::string hashedPassword = hashPassword(password);
        const char* sql = "INSERT INTO users (name, email, password) VALUES (?, ?, ?);";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        bool success = (rc == SQLITE_DONE);
        
        if (!success) {
            std::cerr << "Registration error: " << sqlite3_errmsg(db) << std::endl;
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return success;
    }
    
    bool userExists(const std::string& email) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(dbName.c_str(), &db);
        if (rc) return false;
        
        const char* sql = "SELECT COUNT(*) FROM users WHERE email = ?;";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
        
        bool exists = false;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            exists = sqlite3_column_int(stmt, 0) > 0;
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return exists;
    }
};

int main() {
    std::cout << "User Registration System - Test Cases\\n\\n";
    Task46 system;
    
    // Test Case 1
    std::cout << "Test Case 1: Register new user\\n";
    bool result1 = system.registerUser("John Doe", "john@example.com", "password123");
    std::cout << "Registration successful: " << (result1 ? "true" : "false") << "\\n";
    std::cout << "User exists: " << (system.userExists("john@example.com") ? "true" : "false") << "\\n\\n";
    
    // Test Case 2
    std::cout << "Test Case 2: Register another user\\n";
    bool result2 = system.registerUser("Jane Smith", "jane@example.com", "securePass456");
    std::cout << "Registration successful: " << (result2 ? "true" : "false") << "\\n";
    std::cout << "User exists: " << (system.userExists("jane@example.com") ? "true" : "false") << "\\n\\n";
    
    // Test Case 3
    std::cout << "Test Case 3: Try to register duplicate email\\n";
    bool result3 = system.registerUser("John Another", "john@example.com", "newpassword");
    std::cout << "Registration successful: " << (result3 ? "true" : "false") << "\\n\\n";
    
    // Test Case 4
    std::cout << "Test Case 4: Register user with special characters\\n";
    bool result4 = system.registerUser("Mike O'Brien", "mike.obrien@example.com", "P@ssw0rd!");
    std::cout << "Registration successful: " << (result4 ? "true" : "false") << "\\n";
    std::cout << "User exists: " << (system.userExists("mike.obrien@example.com") ? "true" : "false") << "\\n\\n";
    
    // Test Case 5
    std::cout << "Test Case 5: Register user with long name\\n";
    bool result5 = system.registerUser("Alexander Christopher Wellington", "alex@example.com", "myPassword789");
    std::cout << "Registration successful: " << (result5 ? "true" : "false") << "\\n";
    std::cout << "User exists: " << (system.userExists("alex@example.com") ? "true" : "false") << "\\n";
    
    return 0;
}
