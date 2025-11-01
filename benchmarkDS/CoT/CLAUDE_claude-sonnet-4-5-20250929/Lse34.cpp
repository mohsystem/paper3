
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <vector>
#include <sstream>
#include <iomanip>

class Lse34 {
private:
    static const std::string DB_NAME;
    
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string ret;
        int i = 0;
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
                    ret += base64_chars[char_array_4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(int j = i; j < 3; j++)
                char_array_3[j] = '\\0';
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (int j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];
            
            while(i++ < 3)
                ret += '=';
        }
        
        return ret;
    }
    
    static std::vector<unsigned char> base64_decode(const std::string& encoded_string) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::vector<unsigned char> ret;
        int i = 0;
        int in_len = encoded_string.size();
        unsigned char char_array_4[4], char_array_3[3];
        
        for(int pos = 0; pos < in_len && encoded_string[pos] != '='; pos++) {
            char_array_4[i++] = base64_chars.find(encoded_string[pos]);
            if (i == 4) {
                char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
                char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
                char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
                
                for (i = 0; i < 3; i++)
                    ret.push_back(char_array_3[i]);
                i = 0;
            }
        }
        
        return ret;
    }
    
public:
    static std::string hashPassword(const std::string& password, const std::vector<unsigned char>& salt) {
        unsigned char hash[32];
        
        PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt.data(), salt.size(),
                          100000, EVP_sha256(),
                          32, hash);
        
        return base64_encode(hash, 32);
    }
    
    static std::vector<unsigned char> generateSalt() {
        std::vector<unsigned char> salt(32);
        RAND_bytes(salt.data(), 32);
        return salt;
    }
    
    static std::string authenticateUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "redirect:/login?error=invalid";
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        if (sqlite3_open("users.db", &db) != SQLITE_OK) {
            std::cerr << "Cannot open database" << std::endl;
            return "redirect:/login?error=invalid";
        }
        
        const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            return "redirect:/login?error=invalid";
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        
        std::string result = "redirect:/login?error=invalid";
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string saltStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            
            std::vector<unsigned char> salt = base64_decode(saltStr);
            std::string inputHash = hashPassword(password, salt);
            
            if (storedHash == inputHash) {
                result = "redirect:/dashboard";
            }
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
    
    static void createUser(const std::string& username, const std::string& password) {
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        if (sqlite3_open("users.db", &db) != SQLITE_OK) {
            return;
        }
        
        std::vector<unsigned char> salt = generateSalt();
        std::string passwordHash = hashPassword(password, salt);
        std::string saltStr = base64_encode(salt.data(), salt.size());
        
        const char* query = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 3, saltStr.c_str(), -1, SQLITE_TRANSIENT);
            
            sqlite3_step(stmt);
            sqlite3_finalize(stmt);
        }
        
        sqlite3_close(db);
    }
    
    static void initDatabase() {
        sqlite3* db;
        
        if (sqlite3_open("users.db", &db) != SQLITE_OK) {
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "username TEXT UNIQUE NOT NULL, "
                         "password_hash TEXT NOT NULL, "
                         "salt TEXT NOT NULL)";
        
        sqlite3_exec(db, sql, nullptr, nullptr, nullptr);
        sqlite3_close(db);
    }
};

const std::string Lse34::DB_NAME = "users.db";

int main() {
    Lse34::initDatabase();
    
    std::cout << "=== Test Cases ===" << std::endl;
    
    // Setup test users
    Lse34::createUser("admin", "SecurePass123!");
    Lse34::createUser("user1", "MyPassword456");
    Lse34::createUser("testuser", "Test@789");
    
    // Test 1: Valid credentials
    std::cout << "Test 1 - Valid login: " 
              << Lse34::authenticateUser("admin", "SecurePass123!") << std::endl;
    
    // Test 2: Invalid password
    std::cout << "Test 2 - Wrong password: " 
              << Lse34::authenticateUser("admin", "WrongPass") << std::endl;
    
    // Test 3: Non-existent user
    std::cout << "Test 3 - Invalid user: " 
              << Lse34::authenticateUser("hacker", "password") << std::endl;
    
    // Test 4: Empty credentials
    std::cout << "Test 4 - Empty input: " 
              << Lse34::authenticateUser("", "") << std::endl;
    
    // Test 5: SQL injection attempt
    std::cout << "Test 5 - SQL injection attempt: " 
              << Lse34::authenticateUser("admin' OR '1'='1", "anything") << std::endl;
    
    return 0;
}
