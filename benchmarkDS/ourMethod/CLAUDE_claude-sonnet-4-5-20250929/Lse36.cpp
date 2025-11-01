
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// Rules#6: Use environment variables for credentials
const char* DB_PATH = std::getenv("DB_PATH") ? std::getenv("DB_PATH") : ":memory:";

// Rules#12: Generate cryptographically secure salt
std::string generateSalt() {
    unsigned char salt[16];
    // Rules#12: Use OpenSSL CSPRNG
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        return "";
    }
    
    // Base64 encode
    char encoded[25];
    EVP_EncodeBlock((unsigned char*)encoded, salt, sizeof(salt));
    return std::string(encoded);
}

// Rules#9, Rules#10: Hash password using PBKDF2-HMAC-SHA256
std::string hashPassword(const std::string& password, const std::string& saltStr) {
    // Base64 decode salt
    unsigned char salt[16];
    int saltLen = EVP_DecodeBlock(salt, (unsigned char*)saltStr.c_str(), saltStr.length());
    
    if (saltLen < 0) {
        return "";
    }
    
    unsigned char hash[32];
    // Rules#9: Use at least 210,000 iterations
    const int iterations = 210000;
    
    // Rules#10: Use PBKDF2-HMAC-SHA256
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt, saltLen,
                          iterations,
                          EVP_sha256(),
                          32, hash) != 1) {
        return "";
    }
    
    // Base64 encode result
    char encoded[45];
    EVP_EncodeBlock((unsigned char*)encoded, hash, 32);
    
    // Rules#7: Clear sensitive data from memory
    OPENSSL_cleanse(hash, sizeof(hash));
    
    return std::string(encoded);
}

// Rules#10: Constant-time comparison to prevent timing attacks
bool constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    
    return CRYPTO_memcmp(a.c_str(), b.c_str(), a.length()) == 0;
}

/**
 * Login function that authenticates user credentials
 * Rules#3: Validate and sanitize all inputs
 */
bool login(const std::string& username, const std::string& password) {
    // Rules#3: Validate inputs
    if (username.empty() || password.empty()) {
        return false;
    }
    
    // Rules#3: Validate username length
    if (username.length() > 255) {
        return false;
    }
    
    // Rules#15: Enforce password policy
    if (password.length() < 12 || password.length() > 128) {
        return false;
    }
    
    // Rules#3: Validate username format to prevent SQL injection
    for (char c : username) {
        if (!isalnum(c) && c != '_' && c != '@' && c != '.' && c != '+' && c != '-') {
            return false;
        }
    }
    
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    bool result = false;
    
    // Open database connection
    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {
        std::cerr << "Authentication failed" << std::endl;
        return false;
    }
    
    // Rules#2: Use parameterized query to prevent SQL injection
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ? LIMIT 1";
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        std::cerr << "Authentication failed" << std::endl;
        return false;
    }
    
    // Bind username parameter
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        const char* salt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        
        if (storedHash && salt) {
            // Rules#11: Use unique salt for password hashing
            std::string computedHash = hashPassword(password, std::string(salt));
            
            // Rules#10: Constant-time comparison
            if (constantTimeEquals(std::string(storedHash), computedHash)) {
                result = true;
            }
        }
    }
    
    // Clean up
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    // Rules#7: Return generic result
    return result;
}

int main() {
    // Test cases
    std::cout << "Test Case 1 - Valid credentials:" << std::endl;
    std::cout << login("user1", "ValidPassword123!") << std::endl;
    
    std::cout << "\\nTest Case 2 - Invalid password:" << std::endl;
    std::cout << login("user1", "WrongPassword") << std::endl;
    
    std::cout << "\\nTest Case 3 - Non-existent user:" << std::endl;
    std::cout << login("nonexistent", "SomePassword123!") << std::endl;
    
    std::cout << "\\nTest Case 4 - Empty inputs:" << std::endl;
    std::cout << login("", "") << std::endl;
    
    std::cout << "\\nTest Case 5 - SQL injection attempt:" << std::endl;
    std::cout << login("admin' OR '1'='1", "password") << std::endl;
    
    return 0;
}
