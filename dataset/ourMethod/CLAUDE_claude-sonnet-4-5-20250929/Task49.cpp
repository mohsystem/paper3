
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <regex>
#include <memory>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Security: Constants for validation and crypto
constexpr size_t MAX_NAME_LENGTH = 100;
constexpr size_t MAX_EMAIL_LENGTH = 255;
constexpr size_t MIN_PASSWORD_LENGTH = 8;
constexpr size_t MAX_PASSWORD_LENGTH = 128;
constexpr size_t SALT_SIZE = 16;
constexpr size_t HASH_SIZE = 32;
constexpr int PBKDF2_ITERATIONS = 210000;

// Security: RAII wrapper for OpenSSL EVP context
struct EVPContext {
    EVP_MD_CTX* ctx;
    EVPContext() : ctx(EVP_MD_CTX_new()) {}
    ~EVPContext() { if (ctx) EVP_MD_CTX_free(ctx); }
    EVPContext(const EVPContext&) = delete;
    EVPContext& operator=(const EVPContext&) = delete;
};

// Security: RAII wrapper for database
class Database {
private:
    sqlite3* db;
    
public:
    Database() : db(nullptr) {}
    
    ~Database() {
        if (db) {
            sqlite3_close(db);
            db = nullptr;
        }
    }
    
    // Security: Initialize database with proper schema
    bool initialize(const std::string& dbPath) {
        // Security: Validate database path to prevent path traversal
        if (dbPath.empty() || dbPath.length() > 255) {
            std::cerr << "Invalid database path" << std::endl;
            return false;
        }
        
        // Security: Check for path traversal patterns
        if (dbPath.find("..") != std::string::npos || 
            dbPath.find("//") != std::string::npos) {
            std::cerr << "Path traversal detected" << std::endl;
            return false;
        }
        
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database" << std::endl;
            return false;
        }
        
        // Security: Create table with appropriate constraints
        const char* sql = 
            "CREATE TABLE IF NOT EXISTS users ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "name TEXT NOT NULL CHECK(length(name) <= 100),"
            "email TEXT NOT NULL UNIQUE CHECK(length(email) <= 255),"
            "password_hash BLOB NOT NULL,"
            "salt BLOB NOT NULL,"
            "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
            ");";
        
        char* errMsg = nullptr;
        rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            std::cerr << "SQL error: " << (errMsg ? errMsg : "unknown") << std::endl;
            if (errMsg) sqlite3_free(errMsg);
            return false;
        }
        
        return true;
    }
    
    sqlite3* get() { return db; }
};

// Security: Generate cryptographically secure random salt
bool generateSalt(unsigned char* salt, size_t size) {
    // Security: Use OpenSSL's CSPRNG\n    if (RAND_bytes(salt, static_cast<int>(size)) != 1) {\n        std::cerr << "Failed to generate random salt" << std::endl;\n        return false;\n    }\n    return true;\n}\n\n// Security: Hash password using PBKDF2-HMAC-SHA256\nbool hashPassword(const std::string& password, const unsigned char* salt,\n                  unsigned char* hash) {\n    // Security: Use PBKDF2 with SHA256 and high iteration count\n    if (PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.length()),\n                          salt, SALT_SIZE,\n                          PBKDF2_ITERATIONS,\n                          EVP_sha256(),\n                          HASH_SIZE, hash) != 1) {\n        std::cerr << "Password hashing failed" << std::endl;\n        return false;\n    }\n    return true;\n}\n\n// Security: Validate name input\nbool validateName(const std::string& name) {\n    // Security: Check length bounds\n    if (name.empty() || name.length() > MAX_NAME_LENGTH) {\n        return false;\n    }\n    \n    // Security: Allow only alphanumeric, spaces, hyphens, apostrophes\n    std::regex namePattern("^[a-zA-Z '-]+$");\n    return std::regex_match(name, namePattern);\n}\n\n// Security: Validate email input\nbool validateEmail(const std::string& email) {\n    // Security: Check length bounds\n    if (email.empty() || email.length() > MAX_EMAIL_LENGTH) {\n        return false;\n    }\n    \n    // Security: Basic email format validation\n    std::regex emailPattern("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}$");\n    return std::regex_match(email, emailPattern);\n}\n\n// Security: Validate password input\nbool validatePassword(const std::string& password) {\n    // Security: Check length bounds\n    if (password.length() < MIN_PASSWORD_LENGTH || \n        password.length() > MAX_PASSWORD_LENGTH) {\n        return false;\n    }\n    return true;\n}\n\n// Security: API endpoint to store user data\nbool storeUserData(Database& db, const std::string& name, \n                   const std::string& email, const std::string& password) {\n    // Security: Validate all inputs before processing\n    if (!validateName(name)) {\n        std::cerr << "Invalid name format" << std::endl;\n        return false;\n    }\n    \n    if (!validateEmail(email)) {\n        std::cerr << "Invalid email format" << std::endl;\n        return false;\n    }\n    \n    if (!validatePassword(password)) {\n        std::cerr << "Invalid password (must be 8-128 characters)" << std::endl;\n        return false;\n    }\n    \n    // Security: Generate unique salt for this user\n    unsigned char salt[SALT_SIZE];\n    if (!generateSalt(salt, SALT_SIZE)) {\n        return false;\n    }\n    \n    // Security: Hash password with salt using PBKDF2\n    unsigned char hash[HASH_SIZE];\n    if (!hashPassword(password, salt, hash)) {\n        // Security: Clear sensitive data\n        OPENSSL_cleanse(hash, HASH_SIZE);\n        return false;\n    }\n    \n    // Security: Use prepared statements to prevent SQL injection\n    sqlite3_stmt* stmt = nullptr;\n    const char* sql = "INSERT INTO users (name, email, password_hash, salt) VALUES (?, ?, ?, ?)";\n    \n    int rc = sqlite3_prepare_v2(db.get(), sql, -1, &stmt, nullptr);\n    if (rc != SQLITE_OK) {\n        std::cerr << "Failed to prepare statement" << std::endl;\n        OPENSSL_cleanse(hash, HASH_SIZE);\n        return false;\n    }\n    \n    // Security: Bind validated parameters\n    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);\n    sqlite3_bind_text(stmt, 2, email.c_str(), -1, SQLITE_TRANSIENT);\n    sqlite3_bind_blob(stmt, 3, hash, HASH_SIZE, SQLITE_TRANSIENT);\n    sqlite3_bind_blob(stmt, 4, salt, SALT_SIZE, SQLITE_TRANSIENT);\n    \n    rc = sqlite3_step(stmt);\n    sqlite3_finalize(stmt);\n    \n    // Security: Clear sensitive data from memory\n    OPENSSL_cleanse(hash, HASH_SIZE);\n    \n    if (rc != SQLITE_DONE) {\n        std::cerr << "Failed to insert user data" << std::endl;\n        return false;\n    }\n    \n    return true;\n}\n\nint main() {\n    // Test cases\n    Database db;\n    \n    // Security: Initialize database in current directory only\n    if (!db.initialize("users.db")) {\n        return 1;\n    }\n    \n    std::cout << "API Endpoint Test Cases:" << std::endl;\n    std::cout << "========================" << std::endl;\n    \n    // Test case 1: Valid user data\n    std::cout << "Test 1: Valid user - ";\n    if (storeUserData(db, "John Doe", "john.doe@example.com", "SecurePass123!")) {\n        std::cout << "SUCCESS" << std::endl;\n    } else {\n        std::cout << "FAILED" << std::endl;\n    }\n    \n    // Test case 2: Another valid user\n    std::cout << "Test 2: Valid user - ";\n    if (storeUserData(db, "Jane Smith", "jane.smith@example.com", "AnotherSecure456")) {\n        std::cout << "SUCCESS" << std::endl;\n    } else {\n        std::cout << "FAILED" << std::endl;\n    }\n    \n    // Test case 3: Invalid email format\n    std::cout << "Test 3: Invalid email - ";\n    if (!storeUserData(db, "Bob Brown", "invalid-email", "Password789")) {\n        std::cout << "CORRECTLY REJECTED" << std::endl;\n    } else {\n        std::cout << "FAILED (should reject)" << std::endl;\n    }\n    \n    // Test case 4: Password too short\n    std::cout << "Test 4: Short password - ";\n    if (!storeUserData(db, "Alice Johnson", "alice@example.com", "short")) {\n        std::cout << "CORRECTLY REJECTED" << std::endl;\n    } else {\n        std::cout << "FAILED (should reject)" << std::endl;\n    }\n    \n    // Test case 5: Invalid name with special characters\n    std::cout << "Test 5: Invalid name - ";\n    if (!storeUserData(db, "User<script>", "user@example.com", "ValidPass123")) {\n        std::cout << "CORRECTLY REJECTED" << std::endl;\n    } else {\n        std::cout << "FAILED (should reject)" << std::endl;
    }
    
    return 0;
}
