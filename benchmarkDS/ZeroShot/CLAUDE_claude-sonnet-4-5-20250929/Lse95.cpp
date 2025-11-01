
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <sstream>
#include <iomanip>

class Lse95 {
private:
    static const std::string DB_NAME;
    
    static std::string base64_encode(const unsigned char* data, size_t len) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string ret;
        int i = 0, j = 0;
        unsigned char char_array_3[3], char_array_4[4];
        
        while (len--) {
            char_array_3[i++] = *(data++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for (i = 0; i < 4; i++)
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for (j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];
            
            while (i++ < 3)
                ret += '=';
        }
        
        return ret;
    }
    
public:
    struct RegistrationResult {
        bool success;
        std::string message;
    };
    
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        if (sqlite3_open("users.db", &db) == SQLITE_OK) {
            const char* sql = 
                "CREATE TABLE IF NOT EXISTS users ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "username TEXT UNIQUE NOT NULL,"
                "password_hash TEXT NOT NULL,"
                "salt TEXT NOT NULL)";
            
            sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
            sqlite3_close(db);
        }
    }
    
    static std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, 16);
        return base64_encode(salt, 16);
    }
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, salt.c_str(), salt.length());
        SHA256_Update(&sha256, password.c_str(), password.length());
        SHA256_Final(hash, &sha256);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    static RegistrationResult registerUser(const std::string& username, const std::string& password) {
        if (username.empty()) {
            return {false, "Username cannot be empty"};
        }
        if (password.length() < 6) {
            return {false, "Password must be at least 6 characters"};
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        if (sqlite3_open("users.db", &db) != SQLITE_OK) {
            return {false, "Database connection failed"};
        }
        
        const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);
            
            int result = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            if (result == SQLITE_DONE) {
                return {true, "User registered successfully"};
            } else if (result == SQLITE_CONSTRAINT) {
                return {false, "Username already exists"};
            }
        }
        
        sqlite3_close(db);
        return {false, "Registration failed"};
    }
    
    static bool verifyLogin(const std::string& username, const std::string& password) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        if (sqlite3_open("users.db", &db) != SQLITE_OK) {
            return false;
        }
        
        const char* sql = "SELECT password_hash, salt FROM users WHERE username = ?";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                std::string storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
                std::string salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
                std::string hashedPassword = hashPassword(password, salt);
                
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                
                return storedHash == hashedPassword;
            }
            
            sqlite3_finalize(stmt);
        }
        
        sqlite3_close(db);
        return false;
    }
};

const std::string Lse95::DB_NAME = "users.db";

int main() {
    Lse95::initializeDatabase();
    
    std::cout << "=== Test Case 1: Valid Registration ===" << std::endl;
    auto result1 = Lse95::registerUser("john_doe", "password123");
    std::cout << result1.message << std::endl;
    
    std::cout << "\\n=== Test Case 2: Duplicate Username ===" << std::endl;
    auto result2 = Lse95::registerUser("john_doe", "anotherpass");
    std::cout << result2.message << std::endl;
    
    std::cout << "\\n=== Test Case 3: Empty Username ===" << std::endl;
    auto result3 = Lse95::registerUser("", "password123");
    std::cout << result3.message << std::endl;
    
    std::cout << "\\n=== Test Case 4: Short Password ===" << std::endl;
    auto result4 = Lse95::registerUser("jane_doe", "123");
    std::cout << result4.message << std::endl;
    
    std::cout << "\\n=== Test Case 5: Valid Login ===" << std::endl;
    Lse95::registerUser("alice", "securepass");
    bool loginSuccess = Lse95::verifyLogin("alice", "securepass");
    std::cout << "Login successful: " << (loginSuccess ? "true" : "false") << std::endl;
    
    bool loginFail = Lse95::verifyLogin("alice", "wrongpass");
    std::cout << "Login with wrong password: " << (loginFail ? "true" : "false") << std::endl;
    
    return 0;
}
