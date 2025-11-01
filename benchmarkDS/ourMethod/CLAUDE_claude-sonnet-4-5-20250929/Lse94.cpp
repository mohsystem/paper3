
#include <iostream>
#include <string>
#include <cstring>
#include <random>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <regex>
#include <vector>

const int SALT_LENGTH = 16; // 128-bit salt
const char* DB_NAME = "users.db";

// Base64 encoding function
std::string base64_encode(const unsigned char* data, size_t len) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    int val = 0;
    int valb = -6;
    
    for (size_t i = 0; i < len; i++) {
        val = (val << 8) + data[i];
        valb += 8;
        while (valb >= 0) {
            result.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (result.size() % 4) {
        result.push_back('=');
    }
    return result;
}

// Generate cryptographically secure random salt
std::vector<unsigned char> generate_salt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    
    // Use OpenSSL's RAND_bytes for cryptographically secure random generation\n    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {\n        throw std::runtime_error("Failed to generate secure random salt");\n    }\n    \n    return salt;\n}\n\n// Hash password with salt using SHA-256\nstd::string hash_password(const std::string& password, const std::vector<unsigned char>& salt) {\n    unsigned char hash[SHA256_DIGEST_LENGTH];\n    \n    // Create SHA-256 hash with salt\n    SHA256_CTX sha256;\n    SHA256_Init(&sha256);\n    SHA256_Update(&sha256, salt.data(), salt.size());\n    SHA256_Update(&sha256, password.c_str(), password.length());\n    SHA256_Final(hash, &sha256);\n    \n    // Combine salt and hash\n    std::vector<unsigned char> combined;\n    combined.insert(combined.end(), salt.begin(), salt.end());\n    combined.insert(combined.end(), hash, hash + SHA256_DIGEST_LENGTH);\n    \n    return base64_encode(combined.data(), combined.size());\n}\n\n// Insert user into database with parameterized query to prevent SQL injection\nbool insert_user(const std::string& username, const std::string& password) {\n    // Validate username: 3-50 characters, alphanumeric and underscore only\n    if (username.empty() || username.length() < 3 || username.length() > 50) {\n        std::cerr << "Invalid username length" << std::endl;\n        return false;\n    }\n    \n    std::regex username_pattern("^[a-zA-Z0-9_]+$");\n    if (!std::regex_match(username, username_pattern)) {\n        std::cerr << "Invalid username format" << std::endl;\n        return false;\n    }\n    \n    // Validate password strength: minimum 8 characters\n    if (password.empty() || password.length() < 8) {\n        std::cerr << "Password must be at least 8 characters" << std::endl;\n        return false;\n    }\n    \n    // Generate unique salt for this user\n    std::vector<unsigned char> salt = generate_salt();\n    \n    // Hash password with salt\n    std::string hashed_password = hash_password(password, salt);\n    \n    sqlite3* db = nullptr;\n    sqlite3_stmt* stmt = nullptr;\n    bool success = false;\n    \n    // Open database connection\n    if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {\n        std::cerr << "Database error occurred" << std::endl;\n        if (db) sqlite3_close(db);\n        return false;\n    }\n    \n    // Prepare parameterized statement to prevent SQL injection\n    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";\n    \n    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {\n        // Bind parameters - prevents SQL injection\n        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);\n        sqlite3_bind_text(stmt, 2, hashed_password.c_str(), -1, SQLITE_TRANSIENT);\n        \n        if (sqlite3_step(stmt) == SQLITE_DONE) {\n            success = true;\n        } else {\n            std::cerr << "Database error occurred" << std::endl;\n        }\n    }\n    \n    // Clean up resources\n    if (stmt) sqlite3_finalize(stmt);\n    if (db) sqlite3_close(db);\n    \n    return success;\n}\n\n// Create users table if it doesn't exist
void initialize_database() {
    sqlite3* db = nullptr;
    
    if (sqlite3_open(DB_NAME, &db) == SQLITE_OK) {
        const char* sql = "CREATE TABLE IF NOT EXISTS users ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "username TEXT UNIQUE NOT NULL, "
                         "password_hash TEXT NOT NULL)";
        
        char* err_msg = nullptr;
        if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
            std::cerr << "Failed to initialize database" << std::endl;
            if (err_msg) {
                sqlite3_free(err_msg);
            }
        }
        sqlite3_close(db);
    }
}

int main() {
    // Initialize database
    initialize_database();
    
    // Test case 1: Valid user registration
    std::cout << "Test 1: " << (insert_user("alice123", "SecurePass123!") ? "PASS" : "FAIL") << std::endl;
    
    // Test case 2: Valid user with different credentials
    std::cout << "Test 2: " << (insert_user("bob_user", "MyP@ssw0rd2024") ? "PASS" : "FAIL") << std::endl;
    
    // Test case 3: Invalid username (too short)
    std::cout << "Test 3: " << (!insert_user("ab", "ValidPass123") ? "PASS" : "FAIL") << std::endl;
    
    // Test case 4: Invalid password (too short)
    std::cout << "Test 4: " << (!insert_user("charlie", "short") ? "PASS" : "FAIL") << std::endl;
    
    // Test case 5: Invalid username (special characters)
    std::cout << "Test 5: " << (!insert_user("user@name", "ValidPass123") ? "PASS" : "FAIL") << std::endl;
    
    return 0;
}
