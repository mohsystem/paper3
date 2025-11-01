
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <regex>
#include <vector>

class Lse97 {
private:
    static const std::string DB_NAME;
    
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
        
        std::string result;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        
        while (length--) {
            char_array_3[i++] = *(buffer++);
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for(i = 0; i < 4; i++)
                    result += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (j = 0; j < i + 1; j++)
                result += base64_chars[char_array_4[j]];
            
            while((i++ < 3))
                result += '=';
        }
        
        return result;
    }
    
    static void initializeDatabase() {
        sqlite3* db;
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        
        if (rc) {
            sqlite3_close(db);
            return;
        }
        
        const char* sql = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "username TEXT UNIQUE NOT NULL,"
            "password_hash TEXT NOT NULL,"
            "salt TEXT NOT NULL,"
            "first_name TEXT NOT NULL,"
            "last_name TEXT NOT NULL,"
            "email TEXT NOT NULL)";
        
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        
        if (errMsg) {
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
    
public:
    struct RegistrationResult {
        bool success;
        std::string message;
        
        RegistrationResult(bool s, const std::string& m) : success(s), message(m) {}
    };
    
    static RegistrationResult registerUser(const std::string& username, 
                                          const std::string& password,
                                          const std::string& firstName, 
                                          const std::string& lastName,
                                          const std::string& email) {
        if (username.empty() || password.empty() || firstName.empty() || 
            lastName.empty() || email.empty()) {
            return RegistrationResult(false, "All fields are required");
        }
        
        std::regex emailRegex("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+$");
        if (!std::regex_match(email, emailRegex)) {
            return RegistrationResult(false, "Invalid email format");
        }
        
        initializeDatabase();
        
        sqlite3* db;
        int rc = sqlite3_open(DB_NAME.c_str(), &db);
        
        if (rc) {
            sqlite3_close(db);
            return RegistrationResult(false, "Database error");
        }
        
        sqlite3_stmt* stmt;
        const char* checkSql = "SELECT COUNT(*) FROM users WHERE username = ?";
        rc = sqlite3_prepare_v2(db, checkSql, -1, &stmt, nullptr);
        
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int count = sqlite3_column_int(stmt, 0);
                if (count > 0) {
                    sqlite3_finalize(stmt);
                    sqlite3_close(db);
                    return RegistrationResult(false, "Username already exists");
                }
            }
            sqlite3_finalize(stmt);
        }
        
        std::string salt = generateSalt();
        std::string passwordHash = hashPassword(password, salt);
        
        const char* insertSql = 
            "INSERT INTO users (username, password_hash, salt, first_name, last_name, email) "
            "VALUES (?, ?, ?, ?, ?, ?)";
        
        rc = sqlite3_prepare_v2(db, insertSql, -1, &stmt, nullptr);
        
        if (rc == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 4, firstName.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 5, lastName.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 6, email.c_str(), -1, SQLITE_STATIC);
            
            rc = sqlite3_step(stmt);
            sqlite3_finalize(stmt);
            
            if (rc == SQLITE_DONE) {
                sqlite3_close(db);
                return RegistrationResult(true, "Registration succeeded");
            }
        }
        
        sqlite3_close(db);
        return RegistrationResult(false, "Database error");
    }
};

const std::string Lse97::DB_NAME = "users.db";

int main() {
    std::cout << "=== User Registration System Tests ===" << std::endl << std::endl;
    
    auto result1 = Lse97::registerUser("john_doe", "SecurePass123!", 
                                       "John", "Doe", "john@example.com");
    std::cout << "Test 1: " << result1.message << std::endl;
    
    auto result2 = Lse97::registerUser("jane_smith", "StrongP@ssw0rd", 
                                       "Jane", "Smith", "jane@example.com");
    std::cout << "Test 2: " << result2.message << std::endl;
    
    auto result3 = Lse97::registerUser("john_doe", "AnotherPass456", 
                                       "John", "Different", "different@example.com");
    std::cout << "Test 3: " << result3.message << std::endl;
    
    auto result4 = Lse97::registerUser("bob_wilson", "MyP@ss123", 
                                       "Bob", "Wilson", "invalid-email");
    std::cout << "Test 4: " << result4.message << std::endl;
    
    auto result5 = Lse97::registerUser("alice_brown", "", 
                                       "Alice", "Brown", "alice@example.com");
    std::cout << "Test 5: " << result5.message << std::endl;
    
    return 0;
}
