
#include <iostream>
#include <string>
#include <cstring>
#include <stdexcept>
#include <openssl/evp.h>
#include <openssl/rand.h>

// Constants for secure password hashing
const int ITERATIONS = 210000;  // PBKDF2 iterations >= 210000 for security
const int KEY_LENGTH = 32;      // 32 bytes = 256 bits
const int SALT_LENGTH = 16;     // 16-byte salt

/**
 * Converts binary data to hexadecimal string
 */
std::string toHex(const unsigned char* data, size_t len) {
    const char* hex = "0123456789abcdef";
    std::string result;
    result.reserve(len * 2);
    for (size_t i = 0; i < len; ++i) {
        result += hex[(data[i] >> 4) & 0xF];
        result += hex[data[i] & 0xF];
    }
    return result;
}

/**
 * Registers a user by hashing their password with a unique salt.
 * 
 * @param username The username for registration
 * @param password The password to be hashed
 * @return A string containing the salt and hash in hex, separated by ':'
 */
std::string registerUser(const std::string& username, const std::string& password) {
    // Validate inputs
    if (username.empty() || username.find_first_not_of(" \\t\\n\\r") == std::string::npos) {
        throw std::invalid_argument("Username cannot be empty");
    }
    if (password.length() < 8) {
        throw std::invalid_argument("Password must be at least 8 characters");
    }
    
    // Generate a cryptographically secure random salt (unique per user)
    unsigned char salt[SALT_LENGTH];
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        throw std::runtime_error("Failed to generate random salt");
    }
    
    // Hash the password using PBKDF2-HMAC-SHA256
    unsigned char hash[KEY_LENGTH];
    if (PKCS5_PBKDF2_HMAC(
            password.c_str(), 
            static_cast<int>(password.length()),
            salt, 
            SALT_LENGTH,
            ITERATIONS,
            EVP_sha256(),
            KEY_LENGTH,
            hash) != 1) {
        throw std::runtime_error("Password hashing failed");
    }
    
    // Convert salt and hash to hex for storage
    std::string saltHex = toHex(salt, SALT_LENGTH);
    std::string hashHex = toHex(hash, KEY_LENGTH);
    
    // Clear sensitive data from memory
    OPENSSL_cleanse(const_cast<char*>(password.data()), password.length());
    
    // Return salt:hash format
    return saltHex + ":" + hashHex;
}

int main() {
    // Test case 1: Valid username and password
    try {
        std::string result1 = registerUser("alice", "SecurePass123!");
        std::cout << "User 'alice' registered. Hash: " << result1.substr(0, 50) << "..." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 2: Different user with different password
    try {
        std::string result2 = registerUser("bob", "AnotherP@ss456");
        std::cout << "User 'bob' registered. Hash: " << result2.substr(0, 50) << "..." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 3: Same password for different user should produce different hash
    try {
        std::string result3 = registerUser("charlie", "SecurePass123!");
        std::cout << "User 'charlie' registered. Hash: " << result3.substr(0, 50) << "..." << std::endl;
        std::cout << "Note: Same password produces different hash due to unique salt" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 4: Minimum password length
    try {
        std::string result4 = registerUser("dave", "MinPass8");
        std::cout << "User 'dave' registered. Hash: " << result4.substr(0, 50) << "..." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    // Test case 5: Long username and password
    try {
        std::string result5 = registerUser("elizabeth_longusername", "VeryLongP@ssw0rd!WithSpecialChars");
        std::cout << "User 'elizabeth_longusername' registered. Hash: " << result5.substr(0, 50) << "..." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    return 0;
}
