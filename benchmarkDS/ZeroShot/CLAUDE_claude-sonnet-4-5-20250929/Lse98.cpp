
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <vector>

class Lse98 {
public:
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
        if (username.empty() || password.empty()) {
            return false;
        }
        
        try {
            std::vector<unsigned char> salt = generateSalt();
            std::string hashedPassword = hashPassword(password, salt);
            std::string saltString = base64_encode(salt.data(), salt.size());
            
            const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
            sqlite3_stmt* stmt;
            
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
                return false;
            }
            
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, saltString.c_str(), -1, SQLITE_TRANSIENT);
            
            int result = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            
            return result == SQLITE_DONE;
        } catch (...) {
            return false;
        }
    }
};

int main() {
    sqlite3* db;
    sqlite3_open(":memory:", &db);
    
    const char* createTable = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL, salt TEXT NOT NULL)";
    sqlite3_exec(db, createTable, nullptr, nullptr, nullptr);
    
    // Test case 1: Valid registration
    std::cout << "Test 1: " << Lse98::registerUser("john_doe", "SecurePass123!", db) << std::endl;
    
    // Test case 2: Another valid registration
    std::cout << "Test 2: " << Lse98::registerUser("jane_smith", "MyP@ssw0rd", db) << std::endl;
    
    // Test case 3: Empty username
    std::cout << "Test 3: " << Lse98::registerUser("", "password123", db) << std::endl;
    
    // Test case 4: Empty password
    std::cout << "Test 4: " << Lse98::registerUser("user123", "", db) << std::endl;
    
    // Test case 5: Both empty
    std::cout << "Test 5: " << Lse98::registerUser("", "", db) << std::endl;
    
    sqlite3_close(db);
    return 0;
}
