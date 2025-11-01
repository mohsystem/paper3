
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <vector>

std::string base64_encode(const unsigned char* buffer, size_t length) {
    static const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int i = 0, j = 0;
    unsigned char char_array_3[3], char_array_4[4];

    while (length--) {
        char_array_3[i++] = *(buffer++);
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

std::vector<unsigned char> generate_salt() {
    std::vector<unsigned char> salt(16);
    RAND_bytes(salt.data(), 16);
    return salt;
}

std::string hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt.data(), salt.size());
    SHA256_Update(&sha256, password.c_str(), password.length());
    SHA256_Final(hash, &sha256);
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

bool insert_user(const std::string& username, const std::string& password, sqlite3* db) {
    try {
        std::vector<unsigned char> salt = generate_salt();
        std::string hashed_password = hash_password(password, salt);
        std::string salt_string = base64_encode(salt.data(), salt.size());
        
        sqlite3_stmt* stmt;
        const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
        
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
            return false;
        }
        
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashed_password.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, salt_string.c_str(), -1, SQLITE_TRANSIENT);
        
        bool success = sqlite3_step(stmt) == SQLITE_DONE;
        sqlite3_finalize(stmt);
        return success;
    } catch (...) {
        return false;
    }
}

int main() {
    sqlite3* db;
    sqlite3_open(":memory:", &db);
    
    const char* create_table = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL, salt TEXT NOT NULL)";
    sqlite3_exec(db, create_table, nullptr, nullptr, nullptr);
    
    std::cout << "Test 1: " << insert_user("user1", "Password123!", db) << std::endl;
    std::cout << "Test 2: " << insert_user("user2", "SecureP@ss456", db) << std::endl;
    std::cout << "Test 3: " << insert_user("user3", "MySecret789#", db) << std::endl;
    std::cout << "Test 4: " << insert_user("user4", "Test@1234", db) << std::endl;
    std::cout << "Test 5: " << insert_user("user5", "Admin$Pass999", db) << std::endl;
    
    sqlite3_close(db);
    return 0;
}
