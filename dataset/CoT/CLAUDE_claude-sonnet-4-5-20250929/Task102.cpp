
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <cstring>
#include <vector>

std::string hashPassword(const std::string& password) {
    unsigned char salt[16];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        throw std::runtime_error("Error generating salt");
    }
    
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, sizeof(salt));
    SHA256_Update(&sha256, password.c_str(), password.length());
    SHA256_Final(hash, &sha256);
    
    std::vector<unsigned char> combined(salt, salt + sizeof(salt));
    combined.insert(combined.end(), hash, hash + SHA256_DIGEST_LENGTH);
    
    std::string result;
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    for (size_t i = 0; i < combined.size(); i += 3) {
        unsigned int val = (combined[i] << 16) | 
                          ((i + 1 < combined.size() ? combined[i + 1] : 0) << 8) | 
                          (i + 2 < combined.size() ? combined[i + 2] : 0);
        result += base64_chars[(val >> 18) & 0x3F];
        result += base64_chars[(val >> 12) & 0x3F];
        result += (i + 1 < combined.size()) ? base64_chars[(val >> 6) & 0x3F] : '=';
        result += (i + 2 < combined.size()) ? base64_chars[val & 0x3F] : '=';
    }
    return result;
}

bool updateUserPassword(const std::string& username, const std::string& newPassword) {
    if (username.empty() || newPassword.empty() || 
        username.length() > 255 || newPassword.length() > 255) {
        return false;
    }
    
    std::string hashedPassword = hashPassword(newPassword);
    
    sqlite3* db;
    if (sqlite3_open("users.db", &db) != SQLITE_OK) {
        std::cerr << "Error opening database" << std::endl;
        return false;
    }
    
    const char* sql = "UPDATE users SET password = ? WHERE username = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement" << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, username.c_str(), -1, SQLITE_TRANSIENT);
    
    int result = sqlite3_step(stmt);
    int rowsAffected = sqlite3_changes(db);
    
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return rowsAffected > 0;
}

int main() {
    std::cout << "Test Case 1: " << updateUserPassword("user1", "SecurePass123!") << std::endl;
    std::cout << "Test Case 2: " << updateUserPassword("user2", "AnotherP@ssw0rd") << std::endl;
    std::cout << "Test Case 3: " << updateUserPassword("admin", "AdminSecure#456") << std::endl;
    std::cout << "Test Case 4: " << updateUserPassword("", "password") << std::endl;
    std::cout << "Test Case 5: " << updateUserPassword("user3", "") << std::endl;
    
    return 0;
}
