
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Security constants
constexpr int MIN_PASSWORD_LENGTH = 8;
constexpr int SALT_SIZE = 16;  // 128 bits
constexpr int PBKDF2_ITERATIONS = 210000;  // Secure minimum
constexpr int HASH_SIZE = 32;  // 256 bits

/**
 * Generates a cryptographically secure random salt
 * Uses OpenSSL RAND_bytes for CSPRNG (CWE-330, CWE-759)
 */
bool generateSalt(unsigned char* salt, int size) {
    if (!salt || size <= 0) {
        return false;
    }
    // RAND_bytes uses a cryptographically secure random number generator
    int result = RAND_bytes(salt, size);
    if (result != 1) {
        return false;
    }
    return true;
}

/**
 * Hashes password using PBKDF2-HMAC-SHA256
 * Uses high iteration count and unique salt (CWE-759, CWE-916)
 */
bool hashPassword(const std::string& password, const unsigned char* salt, 
                  int saltLen, unsigned char* hash, int hashLen) {
    if (!salt || !hash || password.empty() || saltLen <= 0 || hashLen <= 0) {
        return false;
    }
    
    // PBKDF2-HMAC-SHA256 for secure password hashing
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(), 
        password.length(),
        salt, 
        saltLen,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        hashLen,
        hash
    );
    
    return (result == 1);
}

/**
 * Validates username format
 * Prevents injection attacks (CWE-20)
 */
bool validateUsername(const std::string& username) {
    if (username.empty() || username.length() < 3 || username.length() > 50) {
        return false;
    }
    // Allow only alphanumeric and underscore
    std::regex usernamePattern("^[a-zA-Z0-9_]{3,50}$");
    return std::regex_match(username, usernamePattern);
}

/**
 * Validates password meets security requirements
 * Enforces strong password policy (CWE-521)
 */
bool validatePassword(const std::string& password) {
    if (password.length() < MIN_PASSWORD_LENGTH) {
        return false;
    }
    
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    
    for (char c : password) {
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (std::ispunct(c)) hasSpecial = true;
    }
    
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

/**
 * Base64 encoding helper
 */
std::string base64Encode(const unsigned char* data, int len) {
    if (!data || len <= 0) {
        return "";
    }
    
    const char* base64Chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string encoded;
    int i = 0;
    unsigned char array3[3];
    unsigned char array4[4];
    
    while (len--) {
        array3[i++] = *(data++);
        if (i == 3) {
            array4[0] = (array3[0] & 0xfc) >> 2;
            array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
            array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
            array4[3] = array3[2] & 0x3f;
            
            for(i = 0; i < 4; i++)
                encoded += base64Chars[array4[i]];
            i = 0;
        }
    }
    
    if (i) {
        for(int j = i; j < 3; j++)
            array3[j] = '\\0';
            
        array4[0] = (array3[0] & 0xfc) >> 2;
        array4[1] = ((array3[0] & 0x03) << 4) + ((array3[1] & 0xf0) >> 4);
        array4[2] = ((array3[1] & 0x0f) << 2) + ((array3[2] & 0xc0) >> 6);
        
        for (int j = 0; j < i + 1; j++)
            encoded += base64Chars[array4[j]];
            
        while(i++ < 3)
            encoded += '=';
    }
    
    return encoded;
}

/**
 * Registers a user with secure password storage
 */
std::string registerUser(const std::string& username, const std::string& password) {
    // Input validation - reject invalid input early (CWE-20)
    if (!validateUsername(username)) {
        return "Error: Invalid username format";
    }
    
    if (!validatePassword(password)) {
        return "Error: Password must be at least " + std::to_string(MIN_PASSWORD_LENGTH) +
               " characters with uppercase, lowercase, digit, and special character";
    }
    
    // Allocate buffers on stack (automatic cleanup)
    unsigned char salt[SALT_SIZE];
    unsigned char passwordHash[HASH_SIZE];
    unsigned char saltAndHash[SALT_SIZE + HASH_SIZE];
    
    // Initialize all buffers to zero
    std::memset(salt, 0, SALT_SIZE);
    std::memset(passwordHash, 0, HASH_SIZE);
    std::memset(saltAndHash, 0, SALT_SIZE + HASH_SIZE);
    
    try {
        // Generate cryptographically secure salt (CWE-330, CWE-759)
        if (!generateSalt(salt, SALT_SIZE)) {
            // Clear sensitive data before returning
            std::memset(salt, 0, SALT_SIZE);
            return "Error: Registration failed";
        }
        
        // Hash password with PBKDF2-HMAC-SHA256 (CWE-916)
        if (!hashPassword(password, salt, SALT_SIZE, passwordHash, HASH_SIZE)) {
            // Clear sensitive data before returning
            std::memset(salt, 0, SALT_SIZE);
            std::memset(passwordHash, 0, HASH_SIZE);
            return "Error: Registration failed";
        }
        
        // Concatenate salt and hash for storage
        std::memcpy(saltAndHash, salt, SALT_SIZE);
        std::memcpy(saltAndHash + SALT_SIZE, passwordHash, HASH_SIZE);
        
        // Encode to Base64 for safe storage
        std::string storedValue = base64Encode(saltAndHash, SALT_SIZE + HASH_SIZE);
        
        // Clear sensitive data from memory (CWE-244)
        std::memset(salt, 0, SALT_SIZE);
        std::memset(passwordHash, 0, HASH_SIZE);
        std::memset(saltAndHash, 0, SALT_SIZE + HASH_SIZE);
        
        // In production: store username and storedValue in database
        // Never log passwords or hashes (CWE-532)
        return storedValue;
        
    } catch (const std::exception& e) {
        // Clear sensitive data on exception
        std::memset(salt, 0, SALT_SIZE);
        std::memset(passwordHash, 0, HASH_SIZE);
        std::memset(saltAndHash, 0, SALT_SIZE + HASH_SIZE);
        
        // Return generic error (CWE-209)
        return "Error: Registration failed";
    }
}

int main() {
    // Test case 1: Valid registration
    std::cout << "Test 1 - Valid user:" << std::endl;
    std::string result1 = registerUser("testUser123", "SecurePass123!");
    std::cout << (result1.find("Error") == 0 ? result1 : "Success: Hash stored") << std::endl;
    
    // Test case 2: Weak password
    std::cout << "\\nTest 2 - Weak password:" << std::endl;
    std::string result2 = registerUser("user456", "WeakPass123");
    std::cout << result2 << std::endl;
    
    // Test case 3: Invalid username
    std::cout << "\\nTest 3 - Invalid username:" << std::endl;
    std::string result3 = registerUser("ab", "ValidPass123!");
    std::cout << result3 << std::endl;
    
    // Test case 4: Password too short
    std::cout << "\\nTest 4 - Short password:" << std::endl;
    std::string result4 = registerUser("validUser", "Sh0rt!");
    std::cout << result4 << std::endl;
    
    // Test case 5: Another valid registration
    std::cout << "\\nTest 5 - Another valid user:" << std::endl;
    std::string result5 = registerUser("anotherUser_99", "AnotherSecure99#");
    std::cout << (result5.find("Error") == 0 ? result5 : "Success: Hash stored") << std::endl;
    
    return 0;
}
