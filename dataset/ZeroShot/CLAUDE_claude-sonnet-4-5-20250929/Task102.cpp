
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <vector>

std::string base64_encode(const unsigned char* data, size_t length) {
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    int val = 0, valb = -6;
    for (size_t i = 0; i < length; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');
    return result;
}

std::vector<unsigned char> base64_decode(const std::string& encoded) {
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::vector<unsigned char> result;
    int val = 0, valb = -8;
    for (unsigned char c : encoded) {
        if (c == '=') break;
        const char* pos = strchr(base64_chars, c);
        if (pos == nullptr) continue;
        val = (val << 6) + (pos - base64_chars);
        valb += 6;
        if (valb >= 0) {
            result.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    return result;
}

std::string generateSalt() {
    unsigned char salt[16];
    RAND_bytes(salt, 16);
    return base64_encode(salt, 16);
}

std::string hashPassword(const std::string& password, const std::string& salt) {
    std::vector<unsigned char> salt_bytes = base64_decode(salt);
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt_bytes.data(), salt_bytes.size());
    SHA256_Update(&sha256, password.c_str(), password.length());
    SHA256_Final(hash, &sha256);
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

bool updateUserPassword(const std::string& username, const std::string& newPassword, sqlite3* db) {
    if (username.empty() || newPassword.empty()) {
        return false;
    }
    
    std::string salt = generateSalt();
    std::string hashedPassword = hashPassword(newPassword, salt);
    
    const char* sql = "UPDATE users SET password_hash = ?, salt = ? WHERE username = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, salt.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, username.c_str(), -1, SQLITE_TRANSIENT);
    
    int result = sqlite3_step(stmt);
    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    
    return result == SQLITE_DONE && changes > 0;
}

int main() {
    sqlite3* db;
    sqlite3_open(":memory:", &db);
    
    // Create test table
    const char* createTable = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, "
                             "username TEXT UNIQUE NOT NULL, password_hash TEXT, salt TEXT)";
    sqlite3_exec(db, createTable, nullptr, nullptr, nullptr);
    
    // Insert test users
    const char* testUsers[] = {"user1", "user2", "user3", "user4", "user5"};
    for (const char* user : testUsers) {
        std::string sql = "INSERT INTO users (username) VALUES ('" + std::string(user) + "')";
        sqlite3_exec(db, sql.c_str(), nullptr, nullptr, nullptr);
    }
    
    // Test cases
    std::cout << "Test 1: " << updateUserPassword("user1", "SecurePass123!", db) << std::endl;
    std::cout << "Test 2: " << updateUserPassword("user2", "MyP@ssw0rd", db) << std::endl;
    std::cout << "Test 3: " << updateUserPassword("nonexistent", "password", db) << std::endl;
    std::cout << "Test 4: " << updateUserPassword("user3", "", db) << std::endl;
    std::cout << "Test 5: " << updateUserPassword("", "password", db) << std::endl;
    
    sqlite3_close(db);
    return 0;
}
