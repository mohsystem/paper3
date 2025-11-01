
#include <iostream>
#include <string>
#include <sqlite3.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <cstring>

class Lse36 {
private:
    static const char* DB_NAME;
    
    // Base64 encode function
    static std::string base64_encode(const unsigned char* buffer, size_t length) {
        BIO *bio, *b64;
        BUF_MEM *bufferPtr;
        
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new(BIO_s_mem());
        bio = BIO_push(b64, bio);
        
        BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
        BIO_write(bio, buffer, length);
        BIO_flush(bio);
        BIO_get_mem_ptr(bio, &bufferPtr);
        
        std::string result(bufferPtr->data, bufferPtr->length);
        BIO_free_all(bio);
        
        return result;
    }
    
    // Hash password using SHA-256
    static std::string hashPassword(const std::string& password) {
        unsigned char hash[SHA256_DIGEST_LENGTH];
        SHA256((unsigned char*)password.c_str(), password.length(), hash);
        return base64_encode(hash, SHA256_DIGEST_LENGTH);
    }
    
public:
    // Login function with secure password verification
    static bool login(const std::string& username, const std::string& password) {
        // Input validation
        if (username.empty() || password.empty()) {
            std::cout << "Invalid credentials." << std::endl;
            return false;
        }
        
        sqlite3* db;
        sqlite3_stmt* stmt;
        
        // Open database
        if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
            std::cerr << "Database error: " << sqlite3_errmsg(db) << std::endl;
            return false;
        }
        
        // Prepare parameterized query to prevent SQL injection
        const char* query = "SELECT password_hash FROM users WHERE username = ?";
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Query preparation error: " << sqlite3_errmsg(db) << std::endl;
            sqlite3_close(db);
            return false;
        }
        
        // Bind parameters
        sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
        
        bool loginSuccess = false;
        
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char* storedHash = (const char*)sqlite3_column_text(stmt, 0);
            std::string inputHash = hashPassword(password);
            
            // Secure password comparison
            if (storedHash && inputHash == storedHash) {
                std::cout << "Login successful. Redirecting to home page..." << std::endl;
                loginSuccess = true;
            } else {
                std::cout << "Incorrect password." << std::endl;
            }
        } else {
            std::cout << "Username not found." << std::endl;
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        
        return loginSuccess;
    }
    
    // Helper method to create test database and users
    static void setupTestDatabase() {
        sqlite3* db;
        char* errMsg = nullptr;
        
        if (sqlite3_open(DB_NAME, &db) != SQLITE_OK) {
            std::cerr << "Cannot open database" << std::endl;
            return;
        }
        
        sqlite3_exec(db, "DROP TABLE IF EXISTS users", nullptr, nullptr, &errMsg);
        sqlite3_exec(db, "CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT UNIQUE, password_hash TEXT)", 
                     nullptr, nullptr, &errMsg);
        
        // Add test users
        const char* testUsers[][2] = {
            {"admin", "admin123"},
            {"user1", "password1"},
            {"testuser", "test@123"},
            {"john_doe", "secure_pass"},
            {"alice", "alice2024"}
        };
        
        sqlite3_stmt* stmt;
        const char* insertQuery = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
        sqlite3_prepare_v2(db, insertQuery, -1, &stmt, nullptr);
        
        for (int i = 0; i < 5; i++) {
            std::string hash = hashPassword(testUsers[i][1]);
            sqlite3_bind_text(stmt, 1, testUsers[i][0], -1, SQLITE_STATIC);
            sqlite3_bind_text(stmt, 2, hash.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        std::cout << "Test database setup complete.\\n" << std::endl;
    }
};

const char* Lse36::DB_NAME = "users.db";

int main() {
    Lse36::setupTestDatabase();
    
    std::cout << "=== Test Cases ===\\n" << std::endl;
    
    // Test case 1: Valid login
    std::cout << "Test 1 - Valid login:" << std::endl;
    Lse36::login("admin", "admin123");
    std::cout << std::endl;
    
    // Test case 2: Wrong password
    std::cout << "Test 2 - Wrong password:" << std::endl;
    Lse36::login("admin", "wrongpassword");
    std::cout << std::endl;
    
    // Test case 3: Non-existent user
    std::cout << "Test 3 - Non-existent user:" << std::endl;
    Lse36::login("nonexistent", "password");
    std::cout << std::endl;
    
    // Test case 4: Empty credentials
    std::cout << "Test 4 - Empty credentials:" << std::endl;
    Lse36::login("", "");
    std::cout << std::endl;
    
    // Test case 5: Valid login for different user
    std::cout << "Test 5 - Valid login for different user:" << std::endl;
    Lse36::login("alice", "alice2024");
    
    return 0;
}
