
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <sstream>
#include <iomanip>

class Lse93 {
public:
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        static const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string ret;
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

            while((i++ < 3))
                ret += '=';
        }

        return ret;
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

    static bool insertUser(const std::string& username, const std::string& password, sqlite3* db) {
        std::string salt = generateSalt();
        std::string hashedPassword = hashPassword(password, salt);

        std::string sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
        sqlite3_stmt* stmt;

        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }

        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);

        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    }
};

int main() {
    sqlite3* db;
    sqlite3_open(":memory:", &db);

    const char* createTable = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE, password_hash TEXT, salt TEXT)";
    sqlite3_exec(db, createTable, nullptr, nullptr, nullptr);

    // Test case 1
    std::cout << "Test 1: " << Lse93::insertUser("user1", "password123", db) << std::endl;

    // Test case 2
    std::cout << "Test 2: " << Lse93::insertUser("user2", "securePass456", db) << std::endl;

    // Test case 3
    std::cout << "Test 3: " << Lse93::insertUser("user3", "myP@ssw0rd", db) << std::endl;

    // Test case 4
    std::cout << "Test 4: " << Lse93::insertUser("user4", "testPassword789", db) << std::endl;

    // Test case 5
    std::cout << "Test 5: " << Lse93::insertUser("user5", "anotherSecure#Pass", db) << std::endl;

    sqlite3_close(db);
    return 0;
}
