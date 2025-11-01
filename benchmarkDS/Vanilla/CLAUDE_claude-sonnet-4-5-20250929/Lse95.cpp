
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
    std::string db_name;
    
    std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char base64_chars[] = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
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
                
            while(i++ < 3)
                result += '=';
        }
        
        return result;
    }
    
public:
    Lse95(const std::string& dbName = "users.db") : db_name(dbName) {
        setupDatabase();
    }
    
    void setupDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        int rc = sqlite3_open(db_name.c_str(), &db);
        if (rc) {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            return;
        }
        
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                         "username TEXT UNIQUE NOT NULL,"
                         "password TEXT NOT NULL);";
        
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
        
        sqlite3_close(db);
    }
    
    std::string generateSalt() {
        unsigned char salt[16];
        RAND_bytes(salt, 16);
        return base64_encode(salt, 16);
    }
    
    std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string combined = salt + password;
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
    bool registerUser(const std::string& username, const std::string& password) {
        sqlite3* db;
        int rc = sqlite3_open(db_name.c_str(), &db);
        if (rc) {
            return false;
        }
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        std::string storedPassword = hashedPassword + ":" + salt;
        
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO users (username, password) VALUES (?, ?)";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, storedPassword.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return rc == SQLITE_DONE;
    }
    
    bool verifyUser(const std::string& username, const std::string& password) {
        sqlite3* db;
        int rc = sqlite3_open(db_name.c_str(), &db);
        if (rc) {
            return false;
        }
        
        sqlite3_stmt* stmt;
        const char* sql = "SELECT password FROM users WHERE username = ?";
        
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            sqlite3_close(db);
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            std::string storedPassword = (const char*)sqlite3_column_text(stmt, 0);
            size_t pos = storedPassword.find(":");
            std::string storedHash = storedPassword.substr(0, pos);
            std::string salt = storedPassword.substr(pos + 1);
            
            std::string hashedPassword = hashPassword(password, salt);
            
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            
            return hashedPassword == storedHash;
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
};

int main() {
    Lse95 authSystem;
    
    std::cout << "Test Case 1: Register user1" << std::endl;
    std::cout << authSystem.registerUser("user1", "password123") << std::endl;
    
    std::cout << "\\nTest Case 2: Register user2" << std::endl;
    std::cout << authSystem.registerUser("user2", "securePass456") << std::endl;
    
    std::cout << "\\nTest Case 3: Verify user1 with correct password" << std::endl;
    std::cout << authSystem.verifyUser("user1", "password123") << std::endl;
    
    std::cout << "\\nTest Case 4: Verify user1 with incorrect password" << std::endl;
    std::cout << authSystem.verifyUser("user1", "wrongpassword") << std::endl;
    
    std::cout << "\\nTest Case 5: Verify non-existent user" << std::endl;
    std::cout << authSystem.verifyUser("user3", "password789") << std::endl;
    
    return 0;
}
