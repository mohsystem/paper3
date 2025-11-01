
#include <iostream>
#include <string>
#include <regex>
#include <random>
#include <vector>
#include <cstring>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/rand.h>

// Constants
const int SALT_LENGTH = 16; // 128-bit salt
const int USERNAME_MIN = 3;
const int USERNAME_MAX = 50;
const int PASSWORD_MIN = 8;
const int PASSWORD_MAX = 128;

/**
 * Encodes binary data to base64 string
 */
std::string base64_encode(const unsigned char* data, size_t len) {
    static const char* base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string result;
    int i = 0;
    unsigned char char_array_3[3];
    unsigned char char_array_4[4];
    
    while (len--) {
        char_array_3[i++] = *(data++);
        if (i == 3) {
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            char_array_4[3] = char_array_3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                result += base64_chars[char_array_4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(int j = i; j < 3; j++)
            char_array_3[j] = '\\0';
        
        char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
        char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
        char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
        
        for (int j = 0; j < i + 1; j++)
            result += base64_chars[char_array_4[j]];
        
        while(i++ < 3)
            result += '=';
    }
    
    return result;
}

/**
 * Generates a cryptographically secure random salt using OpenSSL.
 * Returns base64-encoded salt string.
 */
std::string generate_salt() {
    unsigned char salt[SALT_LENGTH];
    
    // Use OpenSSL's CSPRNG to generate random bytes
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
    
    return base64_encode(salt, SALT_LENGTH);
}

/**
 * Hashes password with salt using SHA-256.
 * Note: For production, use bcrypt, scrypt, or Argon2id.
 * This implementation demonstrates the salting concept.
 */
std::string hash_password(const std::string& password, const std::string& salt) {
    std::string salted_password = password + salt;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    // Compute SHA-256 hash
    SHA256(reinterpret_cast<const unsigned char*>(salted_password.c_str()),
           salted_password.length(), hash);
    
    return base64_encode(hash, SHA256_DIGEST_LENGTH);
}

/**
 * Validates username to prevent SQL injection and ensure format.
 * Accepts only alphanumeric, underscore, and hyphen.
 * Length between 3 and 50 characters.
 */
bool is_valid_username(const std::string& username) {
    if (username.empty() || username.length() < USERNAME_MIN || 
        username.length() > USERNAME_MAX) {
        return false;
    }
    
    std::regex pattern("^[a-zA-Z0-9_-]+$");
    return std::regex_match(username, pattern);
}

/**
 * Validates password to ensure security requirements.
 * Minimum 8 characters, maximum 128 characters.
 */
bool is_valid_password(const std::string& password) {
    if (password.empty() || password.length() < PASSWORD_MIN || 
        password.length() > PASSWORD_MAX) {
        return false;
    }
    return true;
}

/**
 * Inserts username and hashed password into database.
 * Uses parameterized queries to prevent SQL injection.
 */
bool insert_user(const std::string& username, const std::string& password) {
    // Validate inputs before processing
    if (!is_valid_username(username)) {
        std::cerr << "Invalid username format" << std::endl;
        return false;
    }
    
    if (!is_valid_password(password)) {
        std::cerr << "Invalid password format" << std::endl;
        return false;
    }
    
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    bool success = false;
    
    try {
        // Generate unique salt
        std::string salt = generate_salt();
        
        // Hash password with salt
        std::string hashed_password = hash_password(password, salt);
        
        // Open database connection
        if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
            throw std::runtime_error("Failed to open database");
        }
        
        // Create table if not exists
        const char* create_table_sql = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username TEXT UNIQUE NOT NULL, "
            "password_hash TEXT NOT NULL, "
            "salt TEXT NOT NULL)";
        
        char* err_msg = nullptr;
        if (sqlite3_exec(db, create_table_sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
            if (err_msg) {
                sqlite3_free(err_msg);
            }
            throw std::runtime_error("Failed to create table");
        }
        
        // Prepare parameterized insert statement
        const char* insert_sql = 
            "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
        
        if (sqlite3_prepare_v2(db, insert_sql, -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare statement");
        }
        
        // Bind parameters to prevent SQL injection
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashed_password.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, salt.c_str(), -1, SQLITE_TRANSIENT);
        
        // Execute statement
        int result = sqlite3_step(stmt);
        success = (result == SQLITE_DONE);
        
    } catch (const std::exception& e) {
        std::cerr << "Database error occurred" << std::endl;
        success = false;
    }
    
    // Cleanup resources
    if (stmt) {
        sqlite3_finalize(stmt);
    }
    if (db) {
        sqlite3_close(db);
    }
    
    return success;
}

int main() {
    // Test case 1: Valid username and password
    std::cout << "Test 1: " << insert_user("john_doe", "SecurePass123!") << std::endl;
    
    // Test case 2: Another valid user
    std::cout << "Test 2: " << insert_user("jane-smith", "MyP@ssw0rd2024") << std::endl;
    
    // Test case 3: Invalid username (too short)
    std::cout << "Test 3: " << insert_user("ab", "ValidPassword1") << std::endl;
    
    // Test case 4: Invalid password (too short)
    std::cout << "Test 4: " << insert_user("validuser", "short") << std::endl;
    
    // Test case 5: Valid edge case
    std::cout << "Test 5: " << insert_user("user123", "LongPassword123456") << std::endl;
    
    return 0;
}
