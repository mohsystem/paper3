
#include <iostream>
#include <string>
#include <map>
#include <cstring>
#include <ctime>
#include <random>
#include <openssl/sha.h>
#include <openssl/rand.h>

// Secure storage for user credentials
struct UserCredential {
    unsigned char passwordHash[SHA256_DIGEST_LENGTH];
    unsigned char salt[16]; // 128-bit salt
    int failedAttempts;
    time_t lockoutUntil;
    
    UserCredential() : failedAttempts(0), lockoutUntil(0) {
        memset(passwordHash, 0, SHA256_DIGEST_LENGTH);
        memset(salt, 0, 16);
    }
};

std::map<std::string, UserCredential> userDatabase;

const int MAX_LOGIN_ATTEMPTS = 3;
const int LOCKOUT_TIME_SECONDS = 300; // 5 minutes

// Generate cryptographically secure random salt
void generateSalt(unsigned char* salt, size_t length) {
    if (RAND_bytes(salt, length) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
}

// Hash password with unique salt using SHA-256 and multiple iterations
void hashPassword(const std::string& password, const unsigned char* salt, unsigned char* output) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    
    // Initial hash with salt
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, salt, 16);
    SHA256_Update(&sha256, password.c_str(), password.length());
    SHA256_Final(hash, &sha256);
    
    // Multiple iterations to slow down brute force attacks
    for (int i = 0; i < 10000; i++) {
        SHA256_Init(&sha256);
        SHA256_Update(&sha256, hash, SHA256_DIGEST_LENGTH);
        SHA256_Final(hash, &sha256);
    }
    
    memcpy(output, hash, SHA256_DIGEST_LENGTH);
}

// Constant-time comparison to prevent timing attacks
bool constantTimeCompare(const unsigned char* a, const unsigned char* b, size_t length) {
    unsigned char result = 0;
    for (size_t i = 0; i < length; i++) {
        result |= a[i] ^ b[i];
    }
    return result == 0;
}

// Add user to database with hashed password
void addUser(const std::string& username, const std::string& password) {
    // Input validation
    if (username.empty() || password.empty()) {
        throw std::invalid_argument("Username and password cannot be empty");
    }
    
    // Validate username format (alphanumeric only, max 50 chars)
    if (username.length() > 50) {
        throw std::invalid_argument("Username too long");
    }
    
    for (char c : username) {
        if (!isalnum(static_cast<unsigned char>(c))) {
            throw std::invalid_argument("Invalid username format");
        }
    }
    
    UserCredential cred;
    generateSalt(cred.salt, 16);
    hashPassword(password, cred.salt, cred.passwordHash);
    
    userDatabase[username] = cred;
}

// Verify password
bool verifyPassword(const std::string& username, const std::string& password) {
    // Input validation
    if (username.empty() || password.empty()) {
        return false;
    }
    
    auto it = userDatabase.find(username);
    
    if (it == userDatabase.end()) {
        // Perform dummy hash to prevent timing attacks
        unsigned char dummySalt[16];
        unsigned char dummyHash[SHA256_DIGEST_LENGTH];
        generateSalt(dummySalt, 16);
        hashPassword(password, dummySalt, dummyHash);
        return false;
    }
    
    UserCredential& cred = it->second;
    
    // Check if account is locked
    if (cred.lockoutUntil > time(NULL)) {
        return false;
    }
    
    // Hash provided password with stored salt
    unsigned char providedHash[SHA256_DIGEST_LENGTH];
    hashPassword(password, cred.salt, providedHash);
    
    // Constant-time comparison
    bool matches = constantTimeCompare(providedHash, cred.passwordHash, SHA256_DIGEST_LENGTH);
    
    if (matches) {
        // Reset failed attempts on success
        cred.failedAttempts = 0;
        cred.lockoutUntil = 0;
    } else {
        // Increment failed attempts
        cred.failedAttempts++;
        if (cred.failedAttempts >= MAX_LOGIN_ATTEMPTS) {
            cred.lockoutUntil = time(NULL) + LOCKOUT_TIME_SECONDS;
        }
    }
    
    return matches;
}

// Simulate session page redirect
void redirectToSessionPage(const std::string& username) {
    std::cout << "Login successful! Redirecting to session page..." << std::endl;
    std::cout << "Welcome, " << username << "!" << std::endl;
    // In a real application, create secure session token here
}

// Main login function
std::string login(const std::string& username, const std::string& password) {
    // Input validation
    if (username.empty() || password.empty()) {
        return "Invalid username or password";
    }
    
    // Sanitize username input - allow only alphanumeric
    std::string sanitizedUsername;
    for (char c : username) {
        if (isalnum(static_cast<unsigned char>(c))) {
            sanitizedUsername += c;
        }
    }
    
    if (sanitizedUsername.empty() || sanitizedUsername != username) {
        return "Invalid username or password";
    }
    
    // Length validation
    if (username.length() > 50 || password.length() > 128) {
        return "Invalid username or password";
    }
    
    // Check if account is locked
    auto it = userDatabase.find(sanitizedUsername);
    if (it != userDatabase.end() && it->second.lockoutUntil > time(NULL)) {
        return "Account temporarily locked due to multiple failed attempts";
    }
    
    // Verify credentials
    if (verifyPassword(sanitizedUsername, password)) {
        redirectToSessionPage(sanitizedUsername);
        return "SUCCESS";
    } else {
        return "Invalid username or password";
    }
}

int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    
    // Initialize with test users
    try {
        addUser("admin", "SecurePass123!");
        addUser("user1", "MyPassword456@");
        addUser("testuser", "Test789#Pass");
    } catch (const std::exception& e) {
        std::cerr << "Error initializing users: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "=== Secure Login System Test Cases ===" << std::endl << std::endl;
    
    // Test case 1: Valid login
    std::cout << "Test 1: Valid credentials (admin/SecurePass123!)" << std::endl;
    std::string result1 = login("admin", "SecurePass123!");
    std::cout << "Result: " << result1 << std::endl << std::endl;
    
    // Test case 2: Invalid password
    std::cout << "Test 2: Invalid password" << std::endl;
    std::string result2 = login("admin", "WrongPassword");
    std::cout << "Result: " << result2 << std::endl << std::endl;
    
    // Test case 3: Non-existent user
    std::cout << "Test 3: Non-existent user" << std::endl;
    std::string result3 = login("nonexistent", "password");
    std::cout << "Result: " << result3 << std::endl << std::endl;
    
    // Test case 4: Empty credentials
    std::cout << "Test 4: Empty username" << std::endl;
    std::string result4 = login("", "password");
    std::cout << "Result: " << result4 << std::endl << std::endl;
    
    // Test case 5: Valid login for another user
    std::cout << "Test 5: Valid credentials (user1/MyPassword456@)" << std::endl;
    std::string result5 = login("user1", "MyPassword456@");
    std::cout << "Result: " << result5 << std::endl << std::endl;
    
    return 0;
}
