
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

class Task49 {
private:
    static const std::string DB_PATH;
    static const std::regex EMAIL_PATTERN;
    static const std::regex NAME_PATTERN;
    
public:
    struct APIResponse {
        bool success;
        std::string message;
        
        APIResponse(bool s, const std::string& m) : success(s), message(m) {}
    };
    
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(password.c_str()), 
               password.length(), hash);
        
        std::stringstream ss;
        for(int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(hash[i]);
        }
        return ss.str();
    }
    
    static bool validateInput(const std::string& name, const std::string& email, 
                             const std::string& password) {
        if (name.empty() || email.empty() || password.empty()) {
            return false;
        }
        
        if (name.length() > 100 || email.length() > 255 || password.length() < 8) {
            return false;
        }
        
        if (!std::regex_match(name, NAME_PATTERN)) {
            return false;
        }
        
        if (!std::regex_match(email, EMAIL_PATTERN)) {
            return false;
        }
        
        return true;
    }
    
    static void initializeDatabase() {
        sqlite3* db;
        int rc = sqlite3_open(DB_PATH.c_str(), &db);
        
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "name TEXT NOT NULL,"
                         "email TEXT UNIQUE NOT NULL,"
                         "password_hash TEXT NOT NULL,"
                         "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
        
        char* errMsg;
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    static APIResponse createUser(const std::string& name, const std::string& email, 
                                  const std::string& password) {
        if (!validateInput(name, email, password)) {
            return APIResponse(false, "Invalid input data");
        }
        
        std::string passwordHash = hashPassword(password);
        
        sqlite3* db;
        int rc = sqlite3_open(DB_PATH.c_str(), &db);
        
        if (rc) {
            return APIResponse(false, "Database connection error");
        }
        
        const char* sql = "INSERT INTO users (name, email, password_hash) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return APIResponse(false, "Statement preparation error");
        }
        
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        if (rc == SQLITE_DONE) {
            return APIResponse(true, "User created successfully");
        } else if (rc == SQLITE_CONSTRAINT) {
            return APIResponse(false, "Email already exists");
        } else {
            return APIResponse(false, "Database error");
        }
    }
};

const std::string Task49::DB_PATH = "users.db";
const std::regex Task49::EMAIL_PATTERN("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
const std::regex Task49::NAME_PATTERN("^[A-Za-z\\\\s]{1,100}$");

int main() {
    Task49::initializeDatabase();
    
    std::cout << "=== Test Case 1: Valid user ===" << std::endl;
    auto response1 = Task49::createUser("John Doe", "john@example.com", "SecurePass123");
    std::cout << "Success: " << response1.success << ", Message: " << response1.message << std::endl;
    
    std::cout << "\\n=== Test Case 2: Duplicate email ===" << std::endl;
    auto response2 = Task49::createUser("Jane Doe", "john@example.com", "AnotherPass456");
    std::cout << "Success: " << response2.success << ", Message: " << response2.message << std::endl;
    
    std::cout << "\\n=== Test Case 3: Invalid email ===" << std::endl;
    auto response3 = Task49::createUser("Alice Smith", "invalid-email", "Password789");
    std::cout << "Success: " << response3.success << ", Message: " << response3.message << std::endl;
    
    std::cout << "\\n=== Test Case 4: Short password ===" << std::endl;
    auto response4 = Task49::createUser("Bob Johnson", "bob@example.com", "short");
    std::cout << "Success: " << response4.success << ", Message: " << response4.message << std::endl;
    
    std::cout << "\\n=== Test Case 5: Invalid name with numbers ===" << std::endl;
    auto response5 = Task49::createUser("Charlie123", "charlie@example.com", "ValidPass123");
    std::cout << "Success: " << response5.success << ", Message: " << response5.message << std::endl;
    
    return 0;
}
