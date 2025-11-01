
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex>
#include <cstring>

class Lse91 {
private:
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string ret;
        int i = 0, j = 0;
        unsigned char char_array_3[3], char_array_4[4];

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
            for(j = i; j < 3; j++)
                char_array_3[j] = '\\0';

            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

            for (j = 0; j < i + 1; j++)
                ret += base64_chars[char_array_4[j]];

            while(i++ < 3)
                ret += '=';
        }

        return ret;
    }

    static std::string hashPassword(const std::string& password, const unsigned char* salt) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256_CTX sha256;
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, salt, 16);
        SHA256_Update(&sha256, password.c_str(), password.length());
        SHA256_Final(hash, &sha256);
        
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }

    static void generateSalt(unsigned char* salt) {
        RAND_bytes(salt, 16);
    }

    static bool isValidUsername(const std::string& username) {
        std::regex pattern("^[a-zA-Z0-9_]{3,20}$");
        return std::regex_match(username, pattern);
    }

    static bool isValidPassword(const std::string& password) {
        return password.length() >= 8;
    }

public:
    static bool registerUser(const std::string& username, const std::string& password) {
        if (!isValidUsername(username)) {
            std::cout << "Invalid username format" << std::endl;
            return false;
        }

        if (!isValidPassword(password)) {
            std::cout << "Invalid password format" << std::endl;
            return false;
        }

        sqlite3* db;
        sqlite3_stmt* stmt;
        int rc = sqlite3_open("users.db", &db);

        if (rc) {
            std::cout << "Database error occurred" << std::endl;
            return false;
        }

        const char* create_table = "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL, salt TEXT NOT NULL)";
        sqlite3_exec(db, create_table, 0, 0, 0);

        unsigned char salt[16];
        generateSalt(salt);
        std::string hashedPassword = hashPassword(password, salt);
        std::string saltString = base64_encode(salt, 16);

        const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

        if (rc != SQLITE_OK) {
            std::cout << "Database error occurred" << std::endl;
            sqlite3_close(db);
            return false;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, saltString.c_str(), -1, SQLITE_STATIC);

        rc = sqlite3_step(stmt);

        if (rc == SQLITE_DONE) {
            std::cout << "User registered successfully: " << username << std::endl;
            sqlite3_finalize(stmt);
            sqlite3_close(db);
            return true;
        } else if (rc == SQLITE_CONSTRAINT) {
            std::cout << "Username already exists" << std::endl;
        } else {
            std::cout << "Database error occurred" << std::endl;
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }
};

int main() {
    std::cout << "=== User Registration System ===" << std::endl << std::endl;

    std::cout << "Test 1: Valid registration" << std::endl;
    Lse91::registerUser("john_doe", "SecurePass123");

    std::cout << "\\nTest 2: Duplicate username" << std::endl;
    Lse91::registerUser("john_doe", "AnotherPass456");

    std::cout << "\\nTest 3: Invalid username" << std::endl;
    Lse91::registerUser("ab", "ValidPass789");

    std::cout << "\\nTest 4: Invalid password" << std::endl;
    Lse91::registerUser("jane_doe", "short");

    std::cout << "\\nTest 5: Another valid registration" << std::endl;
    Lse91::registerUser("alice_smith", "MySecurePassword2024");

    return 0;
}
