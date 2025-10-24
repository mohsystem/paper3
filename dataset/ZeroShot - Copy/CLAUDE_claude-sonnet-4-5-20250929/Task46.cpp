
#include <iostream>
#include <string>
#include <regex>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <cstring>

class Task46 {
private:
    static const std::string DB_NAME;
    static const std::regex EMAIL_PATTERN;
    
    static std::string base64_encode(const unsigned char* data, size_t length) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int i = 0, j = 0;
        unsigned char char_array_3[3], char_array_4[4];
        
        while (length--) {
            char_array_3[i++] = *(data++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                for(i = 0; i < 4; i++) result += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        if (i) {
            for(j = i; j < 3; j++) char_array_3[j] = '\\0';
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            for (j = 0; j < i + 1; j++) result += base64_chars[char_array_4[j]];
            while(i++ < 3) result += '=';
        }
        return result;
    }
    
    static std::string base64_decode(const std::string& encoded) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        int in_len = encoded.size();
        int i = 0, j = 0, in_ = 0;
        unsigned char char_array_4[4], char_array_3[3];
        std::string result;
        
        while (in_len-- && (encoded[in_] != '=')) {
            char_array_4[i++] = encoded[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++)
                    char_array_4[i] = base64_chars.find(char_array_4[i]);
                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
                for (i = 0; i < 3; i++) result += char_array_3[i];
                i = 0;
            }
        }
        if (i) {
            for (j = i; j < 4; j++) char_array_4[j] = 0;
            for (j = 0; j < 4; j++)
                char_array_4[j] = base64_chars.find(char_array_4[j]);
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            for (j = 0; j < i - 1; j++) result += char_array_3[j];
        }
        return result;
    }
    
public:
    struct RegistrationResult {
        bool success;
        std::string message;
    };
    
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        if (sqlite3_open(DB_NAME.c_str(), &db) == SQLITE_OK) {
            const char* sql = 
                "CREATE TABLE IF NOT EXISTS users ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "name TEXT NOT NULL,"
                "email TEXT UNIQUE NOT NULL,"
                "password_hash TEXT NOT NULL,"
                "salt TEXT NOT NULL,"
                "created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)";
            
            sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
            if (errMsg) sqlite3_free(errMsg);
            sqlite3_close(db);
        }
    }
    
    static RegistrationResult registerUser(const std::string& name, 
                                          const std::string& email, 
                                          const std::string& password) {
        if (!validateInput(name, email, password)) {
            return {false, "Invalid input parameters"};
        }
        
        if (!validateEmail(email)) {
            return {false, "Invalid email format"};
        }
        
        if (!validatePassword(password)) {
            return {false, "Password must be at least 8 characters long"};
        }
        
        try {
            std::string salt = generateSalt();
            std::string passwordHash = hashPassword(password, salt);
            
            if (insertUser(name, email, passwordHash, salt)) {
                return {true, "User registered successfully"};
            } else {
                return {false, "Email already exists"};
            }
        } catch (const std::exception& e) {
            return {false, "Registration failed: " + std::string(e.what())};
        }
    }
    
private:
    static bool validateInput(const std::string& name, 
                             const std::string& email, 
                             const std::string& password) {
        return !name.empty() && !email.empty() && !password.empty();
    }
    
    static bool validateEmail(const std::string& email) {
        return std::regex_match(email, EMAIL_PATTERN);
    }
    
    static bool validatePassword(const std::string& password) {
        return password.length() >= 8;
    }
    
    static std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, 16);
        return base64_encode(salt, 16);
    }
    
    static std::string hashPassword(const std::string& password, 
                                    const std::string& salt) {
        std::string decodedSalt = base64_decode(salt);
        std::string combined = decodedSalt + password;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256(reinterpret_cast<const unsigned char*>(combined.c_str()), 
               combined.length(), hash);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    static bool insertUser(const std::string& name, const std::string& email,
                          const std::string& passwordHash, const std::string& salt) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        if (sqlite3_open(DB_NAME.c_str(), &db) != SQLITE_OK) {
            return false;
        }
        
        const char* sql = "INSERT INTO users (name, email, password_hash, salt) VALUES (?, ?, ?, ?)";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 4, salt.c_str(), -1, SQLITE_TRANSIENT);
            
            int result = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            return result == SQLITE_DONE;
        }
        
        sqlite3_close(db);
        return false;
    }
};

const std::string Task46::DB_NAME = "users.db";
const std::regex Task46::EMAIL_PATTERN("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");

int main() {
    Task46::initializeDatabase();
    std::cout << "=== User Registration System - Test Cases ===\\n\\n";
    
    // Test Case 1: Valid registration
    auto result1 = Task46::registerUser("John Doe", "john.doe@example.com", "SecurePass123");
    std::cout << "Test 1 - Valid registration:\\n";
    std::cout << "Result: " << (result1.success ? "true" : "false") << " - " << result1.message << "\\n\\n";
    
    // Test Case 2: Duplicate email
    auto result2 = Task46::registerUser("Jane Doe", "john.doe@example.com", "AnotherPass456");
    std::cout << "Test 2 - Duplicate email:\\n";
    std::cout << "Result: " << (result2.success ? "true" : "false") << " - " << result2.message << "\\n\\n";
    
    // Test Case 3: Invalid email format
    auto result3 = Task46::registerUser("Bob Smith", "invalid-email", "Password789");
    std::cout << "Test 3 - Invalid email:\\n";
    std::cout << "Result: " << (result3.success ? "true" : "false") << " - " << result3.message << "\\n\\n";
    
    // Test Case 4: Weak password
    auto result4 = Task46::registerUser("Alice Brown", "alice@example.com", "weak");
    std::cout << "Test 4 - Weak password:\\n";
    std::cout << "Result: " << (result4.success ? "true" : "false") << " - " << result4.message << "\\n\\n";
    
    // Test Case 5: Valid registration with different user
    auto result5 = Task46::registerUser("Charlie Wilson", "charlie@example.com", "StrongPass999");
    std::cout << "Test 5 - Another valid registration:\\n";
    std::cout << "Result: " << (result5.success ? "true" : "false") << " - " << result5.message << "\\n";
    
    return 0;
}
