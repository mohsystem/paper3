
#include <iostream>
#include <string>
#include <memory>
#include <cstring>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#include <cstdlib>

// Constants for security - Rules#1, Rules#13
const int MAX_USERNAME_LENGTH = 50;
const int MAX_PASSWORD_LENGTH = 128;
const int MAX_LOGIN_ATTEMPTS = 5;
const int PBKDF2_ITERATIONS = 210000;  // Rules#6
const int SALT_LENGTH = 16;

// Secure zeroization function - Rules#1: clear sensitive data
void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL) return;
    volatile unsigned char* p = (volatile unsigned char*)ptr;
    while (len--) {
        *p++ = 0;
    }
}

// Generate cryptographically secure random salt - Rules#8, Rules#9
std::string generate_salt() {
    unsigned char salt[SALT_LENGTH];
    // Use OpenSSL CSPRNG - Rules#1, Rules#6
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
    
    // Encode to base64
    std::string result;
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    for (int i = 0; i < SALT_LENGTH; i += 3) {
        unsigned int n = (salt[i] << 16) | (salt[i+1] << 8) | salt[i+2];
        result += base64_chars[(n >> 18) & 63];
        result += base64_chars[(n >> 12) & 63];
        result += base64_chars[(n >> 6) & 63];
        result += base64_chars[n & 63];
    }
    
    return result;
}

// Decode base64 string
std::vector<unsigned char> base64_decode(const std::string& encoded) {
    std::vector<unsigned char> result;
    // Simplified base64 decode - production code should use a library
    const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    for (size_t i = 0; i < encoded.length(); i += 4) {
        unsigned int n = 0;
        for (int j = 0; j < 4 && i + j < encoded.length(); ++j) {
            size_t pos = base64_chars.find(encoded[i + j]);
            if (pos != std::string::npos) {
                n = (n << 6) | pos;
            }
        }
        result.push_back((n >> 16) & 0xFF);
        result.push_back((n >> 8) & 0xFF);
        result.push_back(n & 0xFF);
    }
    
    return result;
}

// Hash password using PBKDF2-HMAC-SHA256 - Rules#6, Rules#7
std::string hash_password(const std::string& password, const std::string& salt_b64) {
    // Decode salt from base64
    std::vector<unsigned char> salt_bytes = base64_decode(salt_b64);
    
    unsigned char hash[32];  // SHA-256 output
    
    // Use PBKDF2-HMAC-SHA256 with at least 210000 iterations - Rules#6
    if (PKCS5_PBKDF2_HMAC(
            password.c_str(), 
            password.length(),
            salt_bytes.data(), 
            salt_bytes.size(),
            PBKDF2_ITERATIONS,
            EVP_sha256(),
            32,
            hash) != 1) {
        throw std::runtime_error("PBKDF2 failed");
    }
    
    // Encode hash to base64
    std::string result;
    const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    for (int i = 0; i < 32; i += 3) {
        unsigned int n = (hash[i] << 16) | (hash[i+1] << 8) | hash[i+2];
        result += base64_chars[(n >> 18) & 63];
        result += base64_chars[(n >> 12) & 63];
        result += base64_chars[(n >> 6) & 63];
        result += base64_chars[n & 63];
    }
    
    // Clear sensitive data - Rules#1
    secure_zero(hash, sizeof(hash));
    
    return result;
}

// Constant-time string comparison - Rules#6
bool constant_time_compare(const std::string& a, const std::string& b) {
    if (a.length() != b.length()) {
        return false;
    }
    
    volatile unsigned char result = 0;
    for (size_t i = 0; i < a.length(); ++i) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

// Validate username format - Rules#1, Rules#13
bool validate_username(const std::string& username) {
    if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    // Only allow alphanumeric, underscore, and hyphen - Rules#1
    for (char c : username) {
        if (!isalnum(c) && c != '_' && c != '-') {
            return false;
        }
    }
    
    return true;
}

// Authenticate user against database - Rules#1: parameterized queries
bool authenticate_user(const std::string& username, const std::string& password) {
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    bool authenticated = false;
    
    // Get database path from environment - Rules#3, Rules#4
    const char* db_path = std::getenv("DB_PATH");
    if (db_path == nullptr) {
        db_path = "users.db";
    }
    
    // Open database connection - Rules#1
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        std::cerr << "Database connection error" << std::endl;
        return false;
    }
    
    // Use parameterized query to prevent SQL injection - Rules#1
    const char* query = "SELECT password_hash, salt FROM users WHERE username = ?";
    
    if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        return false;
    }
    
    // Bind username parameter - Rules#1
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* stored_hash = sqlite3_column_text(stmt, 0);
        const unsigned char* salt = sqlite3_column_text(stmt, 1);
        
        if (stored_hash != nullptr && salt != nullptr) {
            std::string stored_hash_str(reinterpret_cast<const char*>(stored_hash));
            std::string salt_str(reinterpret_cast<const char*>(salt));
            
            // Hash provided password with stored salt - Rules#6, Rules#8
            std::string computed_hash = hash_password(password, salt_str);
            
            // Constant-time comparison to prevent timing attacks - Rules#6
            authenticated = constant_time_compare(stored_hash_str, computed_hash);
        }
    }
    
    // Clean up resources - Rules#1
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return authenticated;
}

// Login function - main entry point
bool login(const std::string& username, const std::string& password) {
    // Input validation - Rules#1, Rules#13
    if (username.empty() || password.empty()) {
        return false;
    }
    
    // Check length constraints - Rules#1, Rules#13
    if (password.length() > MAX_PASSWORD_LENGTH) {
        return false;
    }
    
    // Validate username format - Rules#1
    if (!validate_username(username)) {
        return false;
    }
    
    // Authenticate user
    return authenticate_user(username, password);
}

// Test cases
int main() {
    std::cout << "Login System Test Cases:" << std::endl;
    
    try {
        // Test Case 1: Generate salt
        std::string salt1 = generate_salt();
        std::cout << "Test 1 - Salt generated: " << (salt1.length() > 0) << std::endl;
        
        // Test Case 2: Hash password with salt
        std::string hash1 = hash_password("testpass123", salt1);
        std::cout << "Test 2 - Password hashed: " << (hash1.length() > 0) << std::endl;
        
        // Test Case 3: Verify same password with same salt produces same hash
        std::string hash2 = hash_password("testpass123", salt1);
        std::cout << "Test 3 - Hash consistency: " << (hash1 == hash2) << std::endl;
        
        // Test Case 4: Verify different salt produces different hash
        std::string salt2 = generate_salt();
        std::string hash3 = hash_password("testpass123", salt2);
        std::cout << "Test 4 - Different salt, different hash: " << (hash1 != hash3) << std::endl;
        
        // Test Case 5: Verify different password produces different hash
        std::string hash4 = hash_password("differentpass", salt1);
        std::cout << "Test 5 - Different password, different hash: " << (hash1 != hash4) << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
