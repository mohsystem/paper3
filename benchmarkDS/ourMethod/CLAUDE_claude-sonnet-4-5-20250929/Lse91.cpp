
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sqlite3.h>
#include <regex>
#include <cstdlib>
#include <iomanip>
#include <sstream>

// Constants for PBKDF2 (Rule #8, #9)
const int ITERATIONS = 210000;  // PBKDF2 iterations >= 210000
const int KEY_LENGTH = 32;      // 256-bit derived key
const int SALT_LENGTH = 16;     // 16-byte salt

// Generate cryptographically secure random salt (Rule #11)
bool generate_salt(unsigned char* salt, size_t length) {
    if (!salt || length == 0) return false;
    // Use OpenSSL's CSPRNG (Rule #1, #9)\n    return RAND_bytes(salt, static_cast<int>(length)) == 1;\n}\n\n// Hash password using PBKDF2-HMAC-SHA256 (Rule #8, #9, #10)\nstd::string hash_password(const std::string& password, const unsigned char* salt, size_t salt_len) {\n    if (!salt || salt_len == 0 || password.empty()) return "";\n    \n    unsigned char key[KEY_LENGTH];\n    std::memset(key, 0, KEY_LENGTH);\n    \n    // Use PBKDF2-HMAC-SHA256 for secure key derivation\n    int result = PKCS5_PBKDF2_HMAC(\n        password.c_str(), static_cast<int>(password.length()),\n        salt, static_cast<int>(salt_len),\n        ITERATIONS,\n        EVP_sha256(),\n        KEY_LENGTH,\n        key\n    );\n    \n    if (result != 1) {\n        // Securely clear key material (Rule #1)\n        OPENSSL_cleanse(key, KEY_LENGTH);\n        return "";\n    }\n    \n    // Combine salt and hash for storage\n    std::vector<unsigned char> combined(salt_len + KEY_LENGTH);\n    std::memcpy(combined.data(), salt, salt_len);\n    std::memcpy(combined.data() + salt_len, key, KEY_LENGTH);\n    \n    // Securely clear key material (Rule #1)\n    OPENSSL_cleanse(key, KEY_LENGTH);\n    \n    // Convert to hex string\n    std::stringstream ss;\n    for (size_t i = 0; i < combined.size(); ++i) {\n        ss << std::hex << std::setw(2) << std::setfill('0') 
           << static_cast<int>(combined[i]);
    }
    
    return ss.str();
}

// Validate username format (Rule #3)
bool validate_username(const std::string& username) {
    if (username.empty() || username.length() > 50) return false;
    // Only allow alphanumeric, underscore, and hyphen
    std::regex pattern("^[a-zA-Z0-9_-]+$");
    return std::regex_match(username, pattern);
}

// Validate password requirements (Rule #13)
bool validate_password(const std::string& password) {
    return password.length() >= 12 && password.length() <= 128;
}

// Register user with secure password hashing (Rule #1, #3, #8, #9, #10, #13, #14, #15)
bool register_user(const std::string& username, const std::string& password) {
    // Input validation (Rule #3)
    if (!validate_username(username)) {
        std::cerr << "Invalid username" << std::endl;
        return false;
    }
    
    // Strong password policy (Rule #13)
    if (!validate_password(password)) {
        std::cerr << "Password must be between 12 and 128 characters" << std::endl;
        return false;
    }
    
    // Generate unique salt (Rule #10, #11)
    unsigned char salt[SALT_LENGTH];
    if (!generate_salt(salt, SALT_LENGTH)) {
        std::cerr << "Registration failed" << std::endl;
        return false;
    }
    
    // Hash password with salt (Rule #8, #9)
    std::string hashed_password = hash_password(password, salt, SALT_LENGTH);
    if (hashed_password.empty()) {
        std::cerr << "Registration failed" << std::endl;
        return false;
    }
    
    // Database path from environment (Rule #14, #15)
    const char* db_path = std::getenv("DB_PATH");
    if (!db_path) db_path = ":memory:";
    
    sqlite3* db = nullptr;
    sqlite3_stmt* stmt = nullptr;
    bool success = false;
    
    // Open database connection
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        std::cerr << "Registration failed" << std::endl;
        if (db) sqlite3_close(db);
        return false;
    }
    
    // Create table if not exists
    const char* create_table = 
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT UNIQUE NOT NULL, "
        "password_hash TEXT NOT NULL)";
    
    if (sqlite3_exec(db, create_table, nullptr, nullptr, nullptr) != SQLITE_OK) {
        std::cerr << "Registration failed" << std::endl;
        sqlite3_close(db);
        return false;
    }
    
    // Use parameterized query to prevent SQL injection (Rule #3)
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashed_password.c_str(), -1, SQLITE_TRANSIENT);
        
        int result = sqlite3_step(stmt);
        success = (result == SQLITE_DONE);
        
        if (!success) {
            // Generic error message (Rule #1)
            std::cerr << "Registration failed" << std::endl;
        }
    } else {
        std::cerr << "Registration failed" << std::endl;
    }
    
    // Cleanup
    if (stmt) sqlite3_finalize(stmt);
    sqlite3_close(db);
    
    return success;
}

int main() {
    // Test cases
    std::cout << "Test 1: Valid registration" << std::endl;
    std::cout << std::boolalpha << register_user("testuser1", "SecurePass123!@#") << std::endl;
    
    std::cout << "\\nTest 2: Valid registration with different user" << std::endl;
    std::cout << std::boolalpha << register_user("testuser2", "AnotherSecure456$%^") << std::endl;
    
    std::cout << "\\nTest 3: Invalid - password too short" << std::endl;
    std::cout << std::boolalpha << register_user("testuser3", "short") << std::endl;
    
    std::cout << "\\nTest 4: Invalid - empty username" << std::endl;
    std::cout << std::boolalpha << register_user("", "ValidPassword123!") << std::endl;
    
    std::cout << "\\nTest 5: Invalid - username with special chars" << std::endl;
    std::cout << std::boolalpha << register_user("test@user", "ValidPassword123!") << std::endl;
    
    return 0;
}
