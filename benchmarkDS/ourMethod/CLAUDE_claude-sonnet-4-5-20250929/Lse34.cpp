
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// PBKDF2 parameters for secure key derivation
const int ITERATIONS = 210000;
const int KEY_LENGTH = 32;
const int SALT_LENGTH = 16;

// Secure zeroization of sensitive data
void secure_zero(void* ptr, size_t len) {
    if (ptr == NULL || len == 0) return;
    volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr);
    while (len--) *p++ = 0;
}

// Base64 encoding
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
    if (valb > -6) result.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (result.size() % 4) result.push_back('=');
    
    return result;
}

// Base64 decoding
std::vector<unsigned char> base64_decode(const std::string& encoded) {
    static const unsigned char decode_table[256] = {
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
    int val = 0;
    int valb = -8;
    
    for (unsigned char c : encoded) {
        if (decode_table[c] == 64) break;
        val = (val << 6) + decode_table[c];
        valb += 6;
        if (valb >= 0) {
            result.push_back((val >> valb) & 0xFF);
            valb -= 8;
        }
    }
    
    return result;
}

// Generate cryptographically secure random salt
std::vector<unsigned char> generate_salt() {
    std::vector<unsigned char> salt(SALT_LENGTH);
    // Use OpenSSL's CSPRNG
    if (RAND_bytes(salt.data(), SALT_LENGTH) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
    return salt;
}

// Hash password using PBKDF2-HMAC-SHA256
std::string hash_password(const std::string& password, const std::vector<unsigned char>& salt) {
    unsigned char hash[KEY_LENGTH];
    
    // Use PBKDF2 with SHA256
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt.data(), salt.size(),
                          ITERATIONS, EVP_sha256(),
                          KEY_LENGTH, hash) != 1) {
        secure_zero(hash, KEY_LENGTH);
        throw std::runtime_error("Password hashing failed");
    }
    
    std::string result = base64_encode(hash, KEY_LENGTH);
    secure_zero(hash, KEY_LENGTH);
    
    return result;
}

// Validate input
bool validate_input(const std::string& value, const std::string& field_name) {
    if (value.empty()) {
        std::cerr << "Error: " << field_name << " cannot be empty" << std::endl;
        return false;
    }
    
    if (value.length() < 3 || value.length() > 100) {
        std::cerr << "Error: " << field_name << " must be between 3 and 100 characters" << std::endl;
        return false;
    }
    
    return true;
}

// Initialize database
void initialize_database(const std::string& db_path) {
    sqlite3* db = nullptr;
    
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Database initialization failed" << std::endl;
        if (db) sqlite3_close(db);
        throw std::runtime_error("Database error");
    }
    
    const char* sql = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL, "
        "salt TEXT NOT NULL)";
    
    char* err_msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cerr << "Table creation failed" << std::endl;
        if (err_msg) {
            sqlite3_free(err_msg);
        }
        sqlite3_close(db);
        throw std::runtime_error("Database error");
    }
    
    sqlite3_close(db);
}

// Register user
bool register_user(const std::string& db_path, const std::string& username, const std::string& password) {
    if (!validate_input(username, "Username") || !validate_input(password, "Password")) {
        return false;
    }
    
    std::vector<unsigned char> salt = generate_salt();
    std::string salt_base64 = base64_encode(salt.data(), salt.size());
    std::string password_hash = hash_password(password, salt);
    
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Registration failed" << std::endl;
        if (db) sqlite3_close(db);
        return false;
    }
    
    // Use parameterized query to prevent SQL injection
    const char* sql = "INSERT INTO users (username, password_hash, salt) VALUES (?, ?, ?)";
    sqlite3_stmt* stmt = nullptr;
    
    bool success = false;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, password_hash.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, salt_base64.c_str(), -1, SQLITE_STATIC);
        
        int result = sqlite3_step(stmt);
        if (result == SQLITE_DONE) {
            success = true;
        } else if (result == SQLITE_CONSTRAINT) {
            std::cerr << "Username already exists" << std::endl;
        } else {
            std::cerr << "Registration failed" << std::endl;
        }
    }
    
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

// Authenticate user
std::string authenticate_user(const std::string& db_path, const std::string& username, const std::string& password) {
    if (!validate_input(username, "Username") || !validate_input(password, "Password")) {
        return "login";
    }
    
    sqlite3* db = nullptr;
    if (sqlite3_open(db_path.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Authentication error occurred" << std::endl;
        if (db) sqlite3_close(db);
        return "login";
    }
    
    // Use parameterized query to prevent SQL injection
    const char* sql = "SELECT password_hash, salt FROM users WHERE username = ?";
    sqlite3_stmt* stmt = nullptr;
    std::string result = "login";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* stored_hash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            const char* salt_base64 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            
            if (stored_hash && salt_base64) {
                std::vector<unsigned char> salt = base64_decode(salt_base64);
                std::string provided_hash = hash_password(password, salt);
                
                // Constant-time comparison to prevent timing attacks
                if (provided_hash.length() == std::strlen(stored_hash) &&
                    CRYPTO_memcmp(provided_hash.c_str(), stored_hash, provided_hash.length()) == 0) {
                    result = "dashboard";
                }
            }
        }
    }
    
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return result;
}

int main() {
    const std::string db_path = "users.db";
    
    try {
        initialize_database(db_path);
        
        // Test case 1: Register and authenticate valid user
        std::cout << "Test 1: Valid user registration and login" << std::endl;
        register_user(db_path, "testuser1", "SecurePass123!");
        std::string result1 = authenticate_user(db_path, "testuser1", "SecurePass123!");
        std::cout << "Result: " << result1 << " (Expected: dashboard)" << std::endl;
        
        // Test case 2: Wrong password
        std::cout << "\\nTest 2: Wrong password" << std::endl;
        std::string result2 = authenticate_user(db_path, "testuser1", "WrongPassword");
        std::cout << "Result: " << result2 << " (Expected: login)" << std::endl;
        
        // Test case 3: Non-existent user
        std::cout << "\\nTest 3: Non-existent user" << std::endl;
        std::string result3 = authenticate_user(db_path, "nonexistent", "password123");
        std::cout << "Result: " << result3 << " (Expected: login)" << std::endl;
        
        // Test case 4: Empty username
        std::cout << "\\nTest 4: Empty username" << std::endl;
        std::string result4 = authenticate_user(db_path, "", "password");
        std::cout << "Result: " << result4 << " (Expected: login)" << std::endl;
        
        // Test case 5: Multiple users with unique salts
        std::cout << "\\nTest 5: Multiple users with unique salts" << std::endl;
        register_user(db_path, "testuser2", "AnotherPass456!");
        std::string result5a = authenticate_user(db_path, "testuser2", "AnotherPass456!");
        std::string result5b = authenticate_user(db_path, "testuser1", "SecurePass123!");
        std::cout << "User2 Result: " << result5a << " (Expected: dashboard)" << std::endl;
        std::cout << "User1 Result: " << result5b << " (Expected: dashboard)" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
