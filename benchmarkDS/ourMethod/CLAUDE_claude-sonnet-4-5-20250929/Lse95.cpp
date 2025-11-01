
#include <iostream>
#include <string>
#include <cstring>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <vector>
#include <algorithm>

// Security constants
const int SALT_LENGTH = 16; // 128 bits
const int PBKDF2_ITERATIONS = 210000;
const int HASH_LENGTH = 32; // 256 bits
const char* DB_PATH = "users.db";

// Base64 encoding helper
std::string base64_encode(const unsigned char* data, size_t len) {
    static const char* encoding_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    result.reserve(((len + 2) / 3) * 4);
    
    for (size_t i = 0; i < len; i += 3) {
        unsigned int triple = (data[i] << 16);
        if (i + 1 < len) triple |= (data[i + 1] << 8);
        if (i + 2 < len) triple |= data[i + 2];
        
        result.push_back(encoding_table[(triple >> 18) & 0x3F]);
        result.push_back(encoding_table[(triple >> 12) & 0x3F]);
        result.push_back(i + 1 < len ? encoding_table[(triple >> 6) & 0x3F] : '=');
        result.push_back(i + 2 < len ? encoding_table[triple & 0x3F] : '=');
    }
    return result;
}

// Base64 decoding helper
std::vector<unsigned char> base64_decode(const std::string& input) {
    static const unsigned char decoding_table[256] = {
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
        64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
        64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
        64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };
    
    std::vector<unsigned char> result;
    result.reserve((input.length() * 3) / 4);
    
    for (size_t i = 0; i < input.length(); i += 4) {
        unsigned int sextet_a = decoding_table[static_cast<unsigned char>(input[i])];
        unsigned int sextet_b = decoding_table[static_cast<unsigned char>(input[i + 1])];
        unsigned int sextet_c = decoding_table[static_cast<unsigned char>(input[i + 2])];
        unsigned int sextet_d = decoding_table[static_cast<unsigned char>(input[i + 3])];
        
        unsigned int triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;
        
        result.push_back((triple >> 16) & 0xFF);
        if (input[i + 2] != '=') result.push_back((triple >> 8) & 0xFF);
        if (input[i + 3] != '=') result.push_back(triple & 0xFF);
    }
    return result;
}

// Generate cryptographically secure random salt using OpenSSL
bool generate_salt(unsigned char* salt, int length) {
    // Use OpenSSL's CSPRNG\n    if (RAND_bytes(salt, length) != 1) {\n        return false;\n    }\n    return true;\n}\n\n// Hash password using PBKDF2-HMAC-SHA256\nbool hash_password(const std::string& password, const unsigned char* salt, \n                   unsigned char* hash, int hash_len) {\n    // Use PBKDF2 with SHA-256 and high iteration count\n    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),\n                          salt, SALT_LENGTH,\n                          PBKDF2_ITERATIONS,\n                          EVP_sha256(),\n                          hash_len, hash) != 1) {\n        return false;\n    }\n    return true;\n}\n\n// Validate username - allow only alphanumeric and underscore\nbool validate_username(const std::string& username) {\n    if (username.empty() || username.length() > 100) {\n        return false;\n    }\n    return std::all_of(username.begin(), username.end(), \n        [](char c) { return std::isalnum(c) || c == '_'; });\n}\n\n// Register user with validated inputs\nbool register_user(const std::string& username, const std::string& password) {\n    // Input validation\n    if (!validate_username(username)) {\n        return false;\n    }\n    if (password.length() < 8 || password.length() > 128) {\n        return false;\n    }\n    \n    sqlite3* db = nullptr;\n    \n    // Open database connection\n    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {\n        if (db) sqlite3_close(db);\n        return false;\n    }\n    \n    // Create table if not exists\n    const char* create_table = \n        "CREATE TABLE IF NOT EXISTS users ("\n        "id INTEGER PRIMARY KEY AUTOINCREMENT, "\n        "username TEXT UNIQUE NOT NULL, "\n        "password_hash TEXT NOT NULL, "\n        "salt TEXT NOT NULL)";\n    \n    char* err_msg = nullptr;\n    if (sqlite3_exec(db, create_table, nullptr, nullptr, &err_msg) != SQLITE_OK) {\n        if (err_msg) sqlite3_free(err_msg);\n        sqlite3_close(db);\n        return false;\n    }\n    \n    // Generate unique cryptographically secure salt\n    unsigned char salt[SALT_LENGTH];\n    if (!generate_salt(salt, SALT_LENGTH)) {\n        sqlite3_close(db);\n        return false;\n    }\n    \n    // Hash password with PBKDF2\n    unsigned char hash[HASH_LENGTH];\n    if (!hash_password(password, salt, hash, HASH_LENGTH)) {\n        // Clear sensitive data before returning\n        memset(hash, 0, HASH_LENGTH);\n        memset(salt, 0, SALT_LENGTH);\n        sqlite3_close(db);\n        return false;\n    }\n    \n    // Encode to base64\n    std::string salt_b64 = base64_encode(salt, SALT_LENGTH);\n    std::string hash_b64 = base64_encode(hash, HASH_LENGTH);\n    \n    // Clear sensitive data from memory\n    memset(hash, 0, HASH_LENGTH);\n    memset(salt, 0, SALT_LENGTH);\n    \n    // Use parameterized query to prevent SQL injection\n    const char* insert_sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";\n    sqlite3_stmt* stmt = nullptr;\n    \n    if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {\n        sqlite3_close(db);\n        return false;\n    }\n    \n    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);\n    sqlite3_bind_text(stmt, 2, hash_b64.c_str(), -1, SQLITE_TRANSIENT);\n    sqlite3_bind_text(stmt, 3, salt_b64.c_str(), -1, SQLITE_TRANSIENT);\n    \n    bool success = (sqlite3_step(stmt) == SQLITE_DONE);\n    \n    sqlite3_finalize(stmt);\n    sqlite3_close(db);\n    \n    return success;\n}\n\n// Constant-time comparison to prevent timing attacks\nbool constant_time_compare(const unsigned char* a, const unsigned char* b, size_t len) {\n    unsigned char result = 0;\n    for (size_t i = 0; i < len; i++) {\n        result |= a[i] ^ b[i];\n    }\n    return result == 0;\n}\n\n// Verify user credentials\nbool verify_user(const std::string& username, const std::string& password) {\n    // Input validation\n    if (username.empty() || password.empty()) {\n        return false;\n    }\n    \n    sqlite3* db = nullptr;\n    \n    if (sqlite3_open(DB_PATH, &db) != SQLITE_OK) {\n        if (db) sqlite3_close(db);\n        return false;\n    }\n    \n    // Use parameterized query\n    const char* select_sql = "SELECT password_hash, salt FROM users WHERE username = ?";\n    sqlite3_stmt* stmt = nullptr;\n    \n    if (sqlite3_prepare_v2(db, select_sql, -1, &stmt, nullptr) != SQLITE_OK) {\n        sqlite3_close(db);\n        return false;\n    }\n    \n    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);\n    \n    bool authenticated = false;\n    \n    if (sqlite3_step(stmt) == SQLITE_ROW) {\n        const char* stored_hash_b64 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));\n        const char* stored_salt_b64 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));\n        \n        if (stored_hash_b64 && stored_salt_b64) {\n            // Decode from base64\n            std::vector<unsigned char> stored_hash = base64_decode(stored_hash_b64);\n            std::vector<unsigned char> stored_salt = base64_decode(stored_salt_b64);\n            \n            // Hash provided password with stored salt\n            unsigned char computed_hash[HASH_LENGTH];\n            if (hash_password(password, stored_salt.data(), computed_hash, HASH_LENGTH)) {\n                // Constant-time comparison\n                authenticated = constant_time_compare(stored_hash.data(), computed_hash, HASH_LENGTH);\n            }\n            \n            // Clear sensitive data\n            memset(computed_hash, 0, HASH_LENGTH);\n        }\n    }\n    \n    sqlite3_finalize(stmt);\n    sqlite3_close(db);\n    \n    return authenticated;\n}\n\nint main() {\n    // Test case 1: Valid registration\n    std::cout << "Test 1 - Valid registration: " \n              << (register_user("user1", "StrongP@ss123") ? "true" : "false") << std::endl;\n    \n    // Test case 2: Valid login\n    std::cout << "Test 2 - Valid login: " \n              << (verify_user("user1", "StrongP@ss123") ? "true" : "false") << std::endl;\n    \n    // Test case 3: Invalid login (wrong password)\n    std::cout << "Test 3 - Invalid password: " \n              << (verify_user("user1", "WrongPassword") ? "true" : "false") << std::endl;\n    \n    // Test case 4: Invalid username (SQL injection attempt)\n    std::cout << "Test 4 - SQL injection attempt: " \n              << (register_user("admin' OR '1'='1", "password123") ? "true" : "false") << std::endl;\n    \n    // Test case 5: Weak password (too short)\n    std::cout << "Test 5 - Weak password: " \n              << (register_user("user2", "weak") ? "true" : "false") << std::endl;
    
    return 0;
}
