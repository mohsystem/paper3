
#include <iostream>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <cstring>

class Lse94 {
private:
    static const int SALT_LENGTH = 16;
    
    static std::string base64_encode(const unsigned char* data, size_t len) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string result;
        int i = 0, j = 0;
        unsigned char array3[3], array4[4];
        
        while (len--) {
            array3[i++] = *(data++);
            if (i == 3) {
                array4[0] = (array3[0] & 0xfc) >> 2;
                array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
                array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
                array4[3] = array3[2] & 0x3f;
                for(i = 0; i < 4; i++) result += base64_chars[array4[i]];
                i = 0;
            }
        }
        
        if (i) {
            for(j = i; j < 3; j++) array3[j] = '\\0';
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            for (j = 0; j < i + 1; j++) result += base64_chars[array4[j]];
            while(i++ < 3) result += '=';
        }
        return result;
    }
    
    static std::string base64_decode(const std::string& encoded) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string decoded;
        std::vector<int> vec(256, -1);
        for (int i = 0; i < 64; i++) vec[base64_chars[i]] = i;
        
        int val = 0, valb = -8;
        for (unsigned char c : encoded) {
            if (vec[c] == -1) break;
            val = (val << 6) + vec[c];
            valb += 6;
            if (valb >= 0) {
                decoded.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return decoded;
    }

public:
    static std::string generateSalt() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        unsigned char salt[SALT_LENGTH];
        for (int i = 0; i < SALT_LENGTH; i++) {
            salt[i] = dis(gen);
        }
        return base64_encode(salt, SALT_LENGTH);
    }
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string saltDecoded = base64_decode(salt);
        std::string combined = saltDecoded + password;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    static bool insertUser(const std::string& username, const std::string& password) {
        if (username.empty() || password.length() < 8) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        std::string combinedHash = salt + ":" + hashedPassword;
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        if (sqlite3_open("users.db", &db) != SQLITE_OK) {
            return false;
        }
        
        const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, combinedHash.c_str(), -1, SQLITE_TRANSIENT);
        
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return success;
    }
};

int main() {
    std::cout << "Test 1: " << Lse94::insertUser("user1", "password123") << std::endl;
    std::cout << "Test 2: " << Lse94::insertUser("user2", "securePass456") << std::endl;
    std::cout << "Test 3: " << Lse94::insertUser("user3", "myP@ssw0rd") << std::endl;
    std::cout << "Test 4: " << Lse94::insertUser("", "password123") << std::endl;
    std::cout << "Test 5: " << Lse94::insertUser("user5", "short") << std::endl;
    
    return 0;
}
