
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <regex>
#include <vector>

class Lse93 {
private:
    static std::string base64_encode(const unsigned char* data, size_t len) {
        const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int val = 0, valb = -6;
        
        for (size_t i = 0; i < len; i++) {
            val = (val << 8) + data[i];
            valb += 8;
            while (valb >= 0) {
                result.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (result.size() % 4) result.push_back('=');
        return result;
    }

public:
    static std::string hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, salt.data(), salt.size());
        SHA256_Update(&sha256, password.c_str(), password.length());
        SHA256_Final(hash, &sha256);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    static std::vector<unsigned char> generateSalt() {
        std::vector<unsigned char> salt(16);
        RAND_bytes(salt.data(), 16);
        return salt;
    }
    
    static bool registerUser(const std::string& username, const std::string& password, sqlite3* db) {
        // Input validation
        if (username.empty() || password.length() < 8) {
            std::cout << "Invalid input: username cannot be empty and password must be at least 8 characters" << std::endl;
            return false;
        }
        
        // Validate username format
        std::regex username_pattern("^[a-zA-Z0-9_]{3,20}$");
        if (!std::regex_match(username, username_pattern)) {
            std::cout << "Invalid username format" << std::endl;
            return false;
        }
        
        try {
            // Generate salt and hash password
            std::vector<unsigned char> salt = generateSalt();
            std::string hashedPassword = hashPassword(password, salt);
            std::string saltString = base64_encode(salt.data(), salt.size());
            
            // Use parameterized query to prevent SQL injection
            const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
            sqlite3_stmt* stmt;
            
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
                std::cout << "Database error: " << sqlite3_errmsg(db) << std::endl;
                return false;
            }
            
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, saltString.c_str(), -1, SQLITE_TRANSIENT);
            
            int result = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            
            return result == SQLITE_DONE;
        } catch (...) {
            std::cout << "Error during registration" << std::endl;
            return false;
        }
    }
    
    static void initializeDatabase(sqlite3* db) {
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "username TEXT UNIQUE NOT NULL, "
                         "password_hash TEXT NOT NULL, "
                         "salt TEXT NOT NULL)";
        
        char* errMsg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cout << "Error creating table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }
};

int main() {
    sqlite3* db;
    
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cout << "Cannot open database" << std::endl;
        return 1;
    }
    
    Lse93::initializeDatabase(db);
    
    // Test cases
    std::cout << "Test Case 1: Valid registration" << std::endl;
    std::cout << "Result: " << Lse93::registerUser("testuser1", "SecurePass123", db) << std::endl;
    
    std::cout << "\\nTest Case 2: Another valid registration" << std::endl;
    std::cout << "Result: " << Lse93::registerUser("alice_99", "MyP@ssw0rd!", db) << std::endl;
    
    std::cout << "\\nTest Case 3: Short password (should fail)" << std::endl;
    std::cout << "Result: " << Lse93::registerUser("testuser2", "short", db) << std::endl;
    
    std::cout << "\\nTest Case 4: Empty username (should fail)" << std::endl;
    std::cout << "Result: " << Lse93::registerUser("", "ValidPass123", db) << std::endl;
    
    std::cout << "\\nTest Case 5: Invalid username characters (should fail)" << std::endl;
    std::cout << "Result: " << Lse93::registerUser("test@user!", "ValidPass123", db) << std::endl;
    
    sqlite3_close(db);
    return 0;
}
