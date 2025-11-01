
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <vector>

std::string base64_encode(const unsigned char* data, size_t len) {
    const char base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string ret;
    int i = 0, j = 0;
    unsigned char char_array_3[3], char_array_4[4];
    
    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            for(i = 0; i < 4; i++) ret += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    if (i) {
        for(j = i; j < 3; j++) char_array_3[j] = '\\0';
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        for (j = 0; j < i + 1; j++) ret += base64_chars[char_array_4[j]];
        while(i++ < 3) ret += '=';
    }
    return ret;
}

std::string hashPassword(const std::string& password) {
    unsigned char salt[16];
    RAND_bytes(salt, 16);
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, 16);
    SHA256_Update(&sha256, password.c_str(), password.length());
    SHA256_Final(hash, &sha256);
    
    std::vector<unsigned char> combined(salt, salt + 16);
    combined.insert(combined.end(), hash, hash + SHA256_DIGEST_LENGTH);
    
    return base64_encode(combined.data(), combined.size());
}

bool insertUser(const std::string& username, const std::string& password) {
    if (username.empty() || password.empty()) {
        return false;
    }
    
    std::string hashedPassword = hashPassword(password);
    
    sqlite3* db;
    char* errMsg = nullptr;
    
    int rc = sqlite3_open("users.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Cannot open database" << std::endl;
        return false;
    }
    
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS users ("
                                "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                                "username TEXT UNIQUE NOT NULL, "
                                "password TEXT NOT NULL)";
    
    rc = sqlite3_exec(db, createTableSQL, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }
    
    const char* insertSQL = "INSERT INTO users (username, password) VALUES (?, ?)";
    sqlite3_stmt* stmt;
    
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement" << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rc == SQLITE_DONE;
}

int main() {
    std::cout << "Test Case 1: " << insertUser("alice", "password123") << std::endl;
    std::cout << "Test Case 2: " << insertUser("bob", "SecureP@ss456") << std::endl;
    std::cout << "Test Case 3: " << insertUser("charlie", "Test!ng789") << std::endl;
    std::cout << "Test Case 4: " << insertUser("", "password") << std::endl;
    std::cout << "Test Case 5: " << insertUser("david", "MyP@ssw0rd") << std::endl;
    
    return 0;
}
