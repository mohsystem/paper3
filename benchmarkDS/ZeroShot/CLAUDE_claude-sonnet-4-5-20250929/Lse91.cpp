
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <vector>

class Lse91 {
private:
    static const char* DB_NAME;
    
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
    
public:
    struct RegistrationResult {
        bool success;
        std::string message;
    };
    
    static void initializeDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "username TEXT UNIQUE NOT NULL,"
                         "password_hash TEXT NOT NULL,"
                         "salt TEXT NOT NULL)";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
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
        if (username.empty() || username.find_first_not_of(' ') == std::string::npos) {
            return {false, "Username cannot be empty"};
        }
        if (password.length() < 8) {
            return {false, "Password must be at least 8 characters"};
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        int rc = sqlite3_open(DB_NAME, &db);
        if (rc) {
            return {false, "Database connection failed"};
        }
        
        const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);
            
            rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            if (rc == SQLITE_DONE) {
                return {true, "User registered successfully"};
            } else if (rc == SQLITE_CONSTRAINT) {
                return {false, "Username already exists"};
            }
        }
        
        sqlite3_close(db);
        return {false, "Registration failed"};
    }
};

const char* Lse91::DB_NAME = "users.db";

int main() {
    Lse91::initializeDatabase();
    
    // Test case 1: Valid registration
    auto result1 = Lse91::registerUser("john_doe", "SecurePass123");
    std::cout << "Test 1: " << result1.message << std::endl;
    
    // Test case 2: Duplicate username
    auto result2 = Lse91::registerUser("john_doe", "AnotherPass456");
    std::cout << "Test 2: " << result2.message << std::endl;
    
    // Test case 3: Short password
    auto result3 = Lse91::registerUser("jane_doe", "short");
    std::cout << "Test 3: " << result3.message << std::endl;
    
    // Test case 4: Empty username
    auto result4 = Lse91::registerUser("", "ValidPassword123");
    std::cout << "Test 4: " << result4.message << std::endl;
    
    // Test case 5: Valid registration with special characters
    auto result5 = Lse91::registerUser("alice_smith", "P@ssw0rd!2023");
    std::cout << "Test 5: " << result5.message << std::endl;
    
    return 0;
}
