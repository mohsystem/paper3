
#include <iostream>
#include <string>
#include <cstring>
#include <random>
#include <sstream>
#include <iomanip>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <sqlite3.h>

// Constants for secure password hashing
const int SALT_LENGTH = 16;  // 128 bits
const int ITERATIONS = 210000;  // OWASP recommendation for PBKDF2
const int KEY_LENGTH = 32;  // 256 bits

// Generate cryptographically secure random salt - CWE-759, CWE-330
std::string generateSalt() {
    unsigned char salt[SALT_LENGTH];
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        return "";
    }
    
    std::stringstream ss;
    for (int i = 0; i < SALT_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)salt[i];
    }
    return ss.str();
}

// Hash password using PBKDF2-HMAC-SHA256 - secure KDF - CWE-327
std::string hashPassword(const std::string& password, const std::string& saltHex) {
    // Convert hex salt to bytes
    unsigned char salt[SALT_LENGTH];
    for (size_t i = 0; i < SALT_LENGTH; i++) {
        sscanf(saltHex.substr(i * 2, 2).c_str(), "%02hhx", &salt[i]);
    }
    
    unsigned char hash[KEY_LENGTH];
    
    // Use PBKDF2 with SHA-256
    if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),
                          salt, SALT_LENGTH,
                          ITERATIONS,
                          EVP_sha256(),
                          KEY_LENGTH, hash) != 1) {
        return "";
    }
    
    // Combine salt and hash
    std::stringstream ss;
    ss << saltHex;
    for (int i = 0; i < KEY_LENGTH; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    
    // Securely clear sensitive data - CWE-14
    explicit_bzero(hash, KEY_LENGTH);
    
    return ss.str();
}

// Validate password policy - CWE-521
bool validatePasswordPolicy(const std::string& password) {
    if (password.empty() || password.length() < 12 || password.length() > 128) {
        return false;
    }
    
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    
    for (char c : password) {
        if (isupper(c)) hasUpper = true;
        if (islower(c)) hasLower = true;
        if (isdigit(c)) hasDigit = true;
        if (strchr("!@#$%^&*()_+-=[]{}; ':\\"\\\\|,.<>/?", c)) hasSpecial = true;
    }
    
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

// Register user with secure password hashing
bool registerUser(const std::string& username, const std::string& password, sqlite3* db) {
    // Input validation - CWE-20
    if (username.empty() || username.length() > 100) {
        std::cerr << "Invalid username" << std::endl;
        return false;
    }
    
    // Validate password policy - CWE-521
    if (!validatePasswordPolicy(password)) {
        std::cerr << "Password must be 12-128 characters with uppercase, lowercase, digit, and special character" << std::endl;
        return false;
    }
    
    // Generate unique salt for this user - CWE-759, CWE-330
    std::string salt = generateSalt();
    if (salt.empty()) {
        std::cerr << "Failed to generate salt" << std::endl;
        return false;
    }
    
    // Hash password with salt using secure KDF - CWE-327
    std::string hashedPassword = hashPassword(password, salt);
    if (hashedPassword.empty()) {
        std::cerr << "Failed to hash password" << std::endl;
        return false;
    }
    
    // Use prepared statement to prevent SQL injection - CWE-89
    const char* sql = "INSERT INTO users (username, password_hash) VALUES (?, ?)";
    sqlite3_stmt* stmt = nullptr;
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        std::cerr << "Registration failed due to system error" << std::endl;
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, hashedPassword.c_str(), -1, SQLITE_TRANSIENT);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    if (rc == SQLITE_CONSTRAINT) {
        std::cerr << "Registration failed: username may already exist" << std::endl;
        return false;
    } else if (rc != SQLITE_DONE) {
        std::cerr << "Registration failed due to system error" << std::endl;
        return false;
    }
    
    return true;
}

int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    
    // Create in-memory database for testing
    sqlite3* db = nullptr;
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) {
        std::cerr << "Cannot open database" << std::endl;
        return 1;
    }
    
    // Create users table
    const char* createTable = "CREATE TABLE users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password_hash TEXT NOT NULL)";
    char* errMsg = nullptr;
    if (sqlite3_exec(db, createTable, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Failed to create table" << std::endl;
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 1;
    }
    
    // Test case 1: Valid registration
    std::cout << "Test 1: " << registerUser("alice", "SecurePass123!@#", db) << std::endl;
    
    // Test case 2: Valid registration with different user
    std::cout << "Test 2: " << registerUser("bob", "MyP@ssw0rd2024!", db) << std::endl;
    
    // Test case 3: Weak password (too short)
    std::cout << "Test 3: " << registerUser("charlie", "Short1!", db) << std::endl;
    
    // Test case 4: Password without special character
    std::cout << "Test 4: " << registerUser("david", "NoSpecialChar123", db) << std::endl;
    
    // Test case 5: Empty username
    std::cout << "Test 5: " << registerUser("", "ValidPass123!@#", db) << std::endl;
    
    sqlite3_close(db);
    EVP_cleanup();
    
    return 0;
}
