
// CPP - Secure password update program with PBKDF2 hashing
// Compiles with: g++ -std=c++17 -o task task.cpp -lsqlite3 -lcrypto -lssl

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <memory>
#include <sqlite3.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Constants for secure password hashing
constexpr int SALT_SIZE = 16;
constexpr int HASH_SIZE = 32;
constexpr int PBKDF2_ITERATIONS = 210000;

class SecurePasswordUpdater {
private:
    sqlite3* db;
    
    // Generate cryptographically secure random salt
    // Uses OpenSSL RAND_bytes for CSPRNG
    bool generateSalt(std::vector<unsigned char>& salt) {
        salt.resize(SALT_SIZE);
        // RAND_bytes returns 1 on success, fail closed on error
        if (RAND_bytes(salt.data(), SALT_SIZE) != 1) {
            std::cerr << "Failed to generate secure random salt" << std::endl;
            return false;
        }
        return true;
    }
    
    // Hash password using PBKDF2-HMAC-SHA256
    // Prevents rainbow table attacks with unique salt and high iteration count
    bool hashPassword(const std::string& password, 
                     const std::vector<unsigned char>& salt,
                     std::vector<unsigned char>& hash) {
        // Validate inputs before processing
        if (password.empty() || salt.size() != SALT_SIZE) {
            std::cerr << "Invalid input for password hashing" << std::endl;
            return false;
        }
        
        hash.resize(HASH_SIZE);
        
        // Use PBKDF2-HMAC-SHA256 with 210k iterations for strong key derivation
        if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                              salt.data(), salt.size(),
                              PBKDF2_ITERATIONS,
                              EVP_sha256(),
                              HASH_SIZE,
                              hash.data()) != 1) {
            // Clear sensitive data on error
            std::fill(hash.begin(), hash.end(), 0);
            std::cerr << "Password hashing failed" << std::endl;
            return false;
        }
        
        return true;
    }
    
    // Convert binary data to hex string for storage
    std::string toHex(const std::vector<unsigned char>& data) {
        const char hex[] = "0123456789abcdef";
        std::string result;
        result.reserve(data.size() * 2);
        for (unsigned char byte : data) {
            result.push_back(hex[byte >> 4]);
            result.push_back(hex[byte & 0x0F]);
        }
        return result;
    }
    
    // Initialize database with secure schema
    bool initDatabase() {
        const char* createTable = 
            "CREATE TABLE IF NOT EXISTS users ("
            "username TEXT PRIMARY KEY NOT NULL, "
            "password_hash TEXT NOT NULL, "
            "salt TEXT NOT NULL);";
        
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg);
        
        if (rc != SQLITE_OK) {
            std::cerr << "Database initialization failed: " << errMsg << std::endl;
            sqlite3_free(errMsg);
            return false;
        }
        
        return true;
    }
    
public:
    SecurePasswordUpdater() : db(nullptr) {}
    
    ~SecurePasswordUpdater() {
        if (db) {
            sqlite3_close(db);
        }
    }
    
    // Initialize database connection
    bool initialize(const std::string& dbPath) {
        // Validate database path to prevent path traversal
        if (dbPath.empty() || dbPath.find("..") != std::string::npos) {
            std::cerr << "Invalid database path" << std::endl;
            return false;
        }
        
        int rc = sqlite3_open(dbPath.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Cannot open database: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        return initDatabase();
    }
    
    // Update user password with secure hashing
    bool updatePassword(const std::string& username, const std::string& newPassword) {
        // Validate inputs - treat all inputs as untrusted
        if (username.empty() || username.length() > 255) {
            std::cerr << "Invalid username length" << std::endl;
            return false;
        }
        
        if (newPassword.empty() || newPassword.length() < 8 || newPassword.length() > 128) {
            std::cerr << "Password must be between 8 and 128 characters" << std::endl;
            return false;
        }
        
        // Generate unique salt for this password
        std::vector<unsigned char> salt;
        if (!generateSalt(salt)) {
            return false;
        }
        
        // Hash password with salt using PBKDF2
        std::vector<unsigned char> hash;
        if (!hashPassword(newPassword, salt, hash)) {
            // Clear sensitive data before returning
            std::fill(salt.begin(), salt.end(), 0);
            return false;
        }
        
        // Convert to hex for storage
        std::string hashHex = toHex(hash);
        std::string saltHex = toHex(salt);
        
        // Clear sensitive data from memory
        std::fill(hash.begin(), hash.end(), 0);
        std::fill(salt.begin(), salt.end(), 0);
        
        // Use parameterized query to prevent SQL injection
        const char* sql = "INSERT OR REPLACE INTO users (username, password_hash, salt) VALUES (?, ?, ?);";
        sqlite3_stmt* stmt = nullptr;
        
        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        // Bind parameters safely
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, hashHex.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, saltHex.c_str(), -1, SQLITE_TRANSIENT);
        
        rc = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        
        if (rc != SQLITE_DONE) {
            std::cerr << "Failed to update password: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        return true;
    }
};

int main() {
    SecurePasswordUpdater updater;
    
    // Initialize with in-memory database for testing
    if (!updater.initialize(":memory:")) {
        return 1;
    }
    
    // Test cases
    struct TestCase {
        std::string username;
        std::string password;
        bool expectedSuccess;
    };
    
    TestCase tests[] = {
        {"alice", "SecurePass123!", true},
        {"bob", "AnotherStr0ng#Pass", true},
        {"charlie", "MyP@ssw0rd2024", true},
        {"", "password", false},  // Empty username should fail
        {"dave", "short", false}  // Too short password should fail
    };
    
    for (const auto& test : tests) {
        bool result = updater.updatePassword(test.username, test.password);
        std::cout << "Test: username='" << test.username << "' - " 
                  << (result == test.expectedSuccess ? "PASS" : "FAIL") << std::endl;
    }
    
    return 0;
}
