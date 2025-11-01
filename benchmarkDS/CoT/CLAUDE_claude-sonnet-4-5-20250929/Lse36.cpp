
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <cstring>
#include <vector>

class Lse36 {
private:
    static const std::string DB_PATH;
    
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string encoded;
        int val = 0, valb = -6;
        
        for (size_t i = 0; i < length; i++) {
            val = (val << 8) + buffer[i];
            valb += 8;
            while (valb >= 0) {
                encoded.push_back(encoding_table[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) encoded.push_back(encoding_table[((val << 8) >> (valb + 8)) & 0x3F]);
        while (encoded.size() % 4) encoded.push_back('=');
        return encoded;
    }
    
    static std::vector<unsigned char> base64_decode(const std::string& input) {
        static const unsigned char decoding_table[256] = {
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
            64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
            64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
        };
        
        std::vector<unsigned char> decoded;
        int val = 0, valb = -8;
        for (unsigned char c : input) {
            if (decoding_table[c] == 64) break;
            val = (val << 6) + decoding_table[c];
            valb += 6;
            if (valb >= 0) {
                decoded.push_back((val >> valb) & 0xFF);
                valb -= 8;
            }
        }
        return decoded;
    }
    
public:
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        std::vector<unsigned char> salt_bytes = base64_decode(salt);
        unsigned char hash[32];
        
        PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt_bytes.data(), salt_bytes.size(),
                          100000, EVP_sha256(), 32, hash);
        
        return base64_encode(hash, 32);
    }
    
    static std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, 16);
        return base64_encode(salt, 16);
    }
    
    static std::string login(const std::string& username, const std::string& password) {
        if (username.empty() || password.empty()) {
            return "Invalid credentials";
        }
        
        sqlite3* db = nullptr;
        sqlite3_stmt* stmt = nullptr;
        
        int rc = sqlite3_open(DB_PATH.c_str(), &db);
        if (rc != SQLITE_OK) {
            if (db) sqlite3_close(db);
            return "Database error: Cannot open database";
        }
        
        const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
        rc = sqlite3_prepare_v2(db, query, -1, &stmt, nullptr);
        
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return "Database error: Cannot prepare statement";
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        
        std::string result;
        rc = sqlite3_step(stmt);
        
        if (rc == SQLITE_ROW) {
            const unsigned char* stored_hash = sqlite3_column_text(stmt, 0);
            const unsigned char* salt = sqlite3_column_text(stmt, 1);
            
            std::string input_hash = hashPassword(password, std::string(reinterpret_cast<const char*>(salt)));
            
            if (input_hash == std::string(reinterpret_cast<const char*>(stored_hash))) {
                result = "Redirect to home page";
            } else {
                result = "Incorrect password";
            }
        } else {
            result = "User not found";
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return result;
    }
};

const std::string Lse36::DB_PATH = "userdb.db";

int main() {
    std::cout << "Test Case 1: Valid credentials" << std::endl;
    std::cout << Lse36::login("user1", "password123") << std::endl;
    
    std::cout << "\\nTest Case 2: Invalid password" << std::endl;
    std::cout << Lse36::login("user1", "wrongpassword") << std::endl;
    
    std::cout << "\\nTest Case 3: Non-existent user" << std::endl;
    std::cout << Lse36::login("nonexistent", "password123") << std::endl;
    
    std::cout << "\\nTest Case 4: Empty username" << std::endl;
    std::cout << Lse36::login("", "password123") << std::endl;
    
    std::cout << "\\nTest Case 5: Empty password" << std::endl;
    std::cout << Lse36::login("user1", "") << std::endl;
    
    return 0;
}
