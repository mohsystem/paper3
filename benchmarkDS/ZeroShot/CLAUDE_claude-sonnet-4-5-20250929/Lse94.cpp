
#include <iostream>
#include <string>
#include <random>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <cstring>
#include <sstream>
#include <iomanip>

class Lse94 {
public:
    static std::string generateSalt() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        
        std::string salt;
        for (int i = 0; i < 16; i++) {
            salt += static_cast<char>(dis(gen));
        }
        return base64Encode(salt);
    }
    
    static std::string hashPassword(const std::string& password, const std::string& salt) {
        std::string decodedSalt = base64Decode(salt);
        std::string combined = decodedSalt + password;
        
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)combined.c_str(), combined.length(), hash);
        
        std::string hashStr((char*)hash, SHA256_DIGEST_LENGTH);
        return base64Encode(hashStr);
    }
    
    static bool insertUser(const std::string& username, const std::string& password) {
        sqlite3* db;
        char* errMsg = nullptr;
        
        if (sqlite3_open("users.db", &db) != SQLITE_OK) {
            std::cerr << "Cannot open database" << std::endl;
            return false;
        }
        
        std::string createTableSQL = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT UNIQUE NOT NULL, "
            "password TEXT NOT NULL, "
            "salt TEXT NOT NULL)";
        
        sqlite3_exec(db, createTableSQL.c_str(), nullptr, nullptr, &errMsg);
        
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);
        
        std::string insertSQL = "INSERT INTO users (username, password, salt) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, insertSQL.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_STATIC);
            
            if (sqlite3_step(stmt) != SQLITE_DONE) {
                std::cerr << "Error inserting user" << std::endl;
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return false;
            }
            sqlite3_finalize(stmt);
        }
        
        sqlite3_close(db);
        return true;
    }
    
private:
    static std::string base64Encode(const std::string& input) {
        static const char* base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string output;
        int val = 0, valb = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                output.push_back(base64_chars[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) output.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
        while (output.size() % 4) output.push_back('=');
        return output;
    }
    
    static std::string base64Decode(const std::string& input) {
        static const std::string base64_chars = 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string output;
        int val = 0, valb = -8;
        for (char c : input) {
            if (c == '=') break;
            size_t pos = base64_chars.find(c);
            if (pos == std::string::npos) continue;
            val = (val << 6) + pos;
            valb += 6;
            if (valb >= 0) {
                output.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return output;
    }
};

int main() {
    std::cout << "Test Case 1: " << Lse94::insertUser("alice", "password123") << std::endl;
    std::cout << "Test Case 2: " << Lse94::insertUser("bob", "securePass456") << std::endl;
    std::cout << "Test Case 3: " << Lse94::insertUser("charlie", "myP@ssw0rd") << std::endl;
    std::cout << "Test Case 4: " << Lse94::insertUser("david", "test1234") << std::endl;
    std::cout << "Test Case 5: " << Lse94::insertUser("eve", "strongPassword!") << std::endl;
    return 0;
}
