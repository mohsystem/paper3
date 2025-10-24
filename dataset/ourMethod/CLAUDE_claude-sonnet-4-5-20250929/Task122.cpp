
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>

// OpenSSL headers for cryptographic operations
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Constants for security parameters
constexpr size_t SALT_SIZE = 16;        // 128-bit salt for PBKDF2
constexpr size_t HASH_SIZE = 32;        // 256-bit hash output
constexpr int PBKDF2_ITERATIONS = 210000; // OWASP recommended minimum
constexpr size_t MIN_PASSWORD_LENGTH = 8;
constexpr size_t MAX_PASSWORD_LENGTH = 128;
constexpr size_t MAX_USERNAME_LENGTH = 64;

// Secure memory cleanup using OPENSSL_cleanse
class SecureString {
private:
    std::vector<unsigned char> data;
public:
    explicit SecureString(size_t size) : data(size, 0) {}
    
    ~SecureString() {
        if (!data.empty()) {
            OPENSSL_cleanse(data.data(), data.size());
        }
    }
    
    unsigned char* get() { return data.data(); }
    size_t size() const { return data.size(); }
    
    // Prevent copying to avoid leaving sensitive data in memory
    SecureString(const SecureString&) = delete;
    SecureString& operator=(const SecureString&) = delete;
};

// Validate username: alphanumeric and underscore only, length check
bool validateUsername(const std::string& username) {
    if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    for (char c : username) {
        if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
            return false;
        }
    }
    return true;
}

// Validate password: length and complexity requirements
bool validatePassword(const std::string& password) {
    if (password.length() < MIN_PASSWORD_LENGTH || 
        password.length() > MAX_PASSWORD_LENGTH) {
        return false;
    }
    
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    
    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        else if (std::islower(static_cast<unsigned char>(c))) hasLower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
        else hasSpecial = true;
    }
    
    // Require at least 3 of 4 character types for strong passwords
    int complexity = hasUpper + hasLower + hasDigit + hasSpecial;
    return complexity >= 3;
}

// Generate cryptographically secure random salt using OpenSSL
bool generateSalt(unsigned char* salt, size_t size) {
    if (salt == nullptr || size == 0) {
        return false;
    }
    
    // Use OpenSSL's CSPRNG - secure random number generator\n    if (RAND_bytes(salt, static_cast<int>(size)) != 1) {\n        return false;\n    }\n    return true;\n}\n\n// Derive hash from password using PBKDF2-HMAC-SHA256\nbool deriveKey(const std::string& password, const unsigned char* salt, \n               size_t saltSize, unsigned char* hash, size_t hashSize) {\n    if (password.empty() || salt == nullptr || hash == nullptr || \n        saltSize == 0 || hashSize == 0) {\n        return false;\n    }\n    \n    // Use PBKDF2 with HMAC-SHA256 for key derivation\n    // High iteration count protects against brute force attacks\n    int result = PKCS5_PBKDF2_HMAC(\n        password.c_str(),\n        static_cast<int>(password.length()),\n        salt,\n        static_cast<int>(saltSize),\n        PBKDF2_ITERATIONS,\n        EVP_sha256(),\n        static_cast<int>(hashSize),\n        hash\n    );\n    \n    return result == 1;\n}\n\n// Convert binary data to hexadecimal string for storage\nstd::string toHex(const unsigned char* data, size_t size) {\n    if (data == nullptr || size == 0) {\n        return "";\n    }\n    \n    std::string hex;\n    hex.reserve(size * 2);\n    \n    const char hexChars[] = "0123456789abcdef";\n    for (size_t i = 0; i < size; ++i) {\n        hex += hexChars[(data[i] >> 4) & 0x0F];\n        hex += hexChars[data[i] & 0x0F];\n    }\n    return hex;\n}\n\n// Check if username already exists in storage file\nbool userExists(const std::string& username, const std::string& filename) {\n    std::ifstream file(filename);\n    if (!file.is_open()) {\n        return false; // File doesn't exist yet
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Format: username:salt:hash
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string storedUser = line.substr(0, pos);
            if (storedUser == username) {
                file.close();
                return true;
            }
        }
    }
    
    file.close();
    return false;
}

// Store user credentials securely
bool storeUser(const std::string& username, const std::string& password, 
               const std::string& filename) {
    // Validate inputs first - fail closed on any validation error
    if (!validateUsername(username)) {
        std::cerr << "Invalid username format" << std::endl;
        return false;
    }
    
    if (!validatePassword(password)) {
        std::cerr << "Password does not meet complexity requirements" << std::endl;
        return false;
    }
    
    // Check if user already exists
    if (userExists(username, filename)) {
        std::cerr << "Username already exists" << std::endl;
        return false;
    }
    
    // Generate unique random salt for this user
    SecureString salt(SALT_SIZE);
    if (!generateSalt(salt.get(), salt.size())) {
        std::cerr << "Failed to generate salt" << std::endl;
        return false;
    }
    
    // Derive password hash using PBKDF2
    SecureString hash(HASH_SIZE);
    if (!deriveKey(password, salt.get(), salt.size(), hash.get(), hash.size())) {
        std::cerr << "Failed to derive key" << std::endl;
        return false;
    }
    
    // Convert to hex for text storage
    std::string saltHex = toHex(salt.get(), salt.size());
    std::string hashHex = toHex(hash.get(), hash.size());
    
    // Store in format: username:salt:hash
    std::ofstream file(filename, std::ios::app);
    if (!file.is_open()) {
        std::cerr << "Failed to open storage file" << std::endl;
        return false;
    }
    
    file << username << ":" << saltHex << ":" << hashHex << std::endl;
    file.close();
    
    // Verify write was successful
    if (file.fail()) {
        std::cerr << "Failed to write to storage file" << std::endl;
        return false;
    }
    
    return true;
}

// Safe input reading with length validation
bool safeGetInput(std::string& input, size_t maxLength, const std::string& prompt) {
    std::cout << prompt;
    
    if (!std::getline(std::cin, input)) {
        return false;
    }
    
    if (input.length() > maxLength) {
        std::cerr << "Input exceeds maximum length" << std::endl;
        return false;
    }
    
    return true;
}

int main() {
    const std::string storageFile = "users.txt";
    
    std::cout << "=== Secure User Signup System ===" << std::endl;
    std::cout << "Password requirements:" << std::endl;
    std::cout << "- Minimum 8 characters" << std::endl;
    std::cout << "- At least 3 of: uppercase, lowercase, digit, special character" << std::endl;
    std::cout << std::endl;
    
    // Test case 1: Valid signup
    {
        std::cout << "\\nTest 1: Valid user signup" << std::endl;
        if (storeUser("alice123", "SecureP@ss1", storageFile)) {
            std::cout << "User registered successfully!" << std::endl;
        } else {
            std::cout << "Registration failed" << std::endl;
        }
    }
    
    // Test case 2: Strong password with different complexity
    {
        std::cout << "\\nTest 2: Another valid user" << std::endl;
        if (storeUser("bob_user", "MyStr0ng#Pass", storageFile)) {
            std::cout << "User registered successfully!" << std::endl;
        } else {
            std::cout << "Registration failed" << std::endl;
        }
    }
    
    // Test case 3: Duplicate username (should fail)
    {
        std::cout << "\\nTest 3: Duplicate username attempt" << std::endl;
        if (storeUser("alice123", "AnotherP@ss1", storageFile)) {
            std::cout << "User registered successfully!" << std::endl;
        } else {
            std::cout << "Registration failed (expected - duplicate)" << std::endl;
        }
    }
    
    // Test case 4: Weak password (should fail)
    {
        std::cout << "\\nTest 4: Weak password attempt" << std::endl;
        if (storeUser("charlie", "weak", storageFile)) {
            std::cout << "User registered successfully!" << std::endl;
        } else {
            std::cout << "Registration failed (expected - weak password)" << std::endl;
        }
    }
    
    // Test case 5: Invalid username characters (should fail)
    {
        std::cout << "\\nTest 5: Invalid username attempt" << std::endl;
        if (storeUser("user@invalid!", "StrongP@ss9", storageFile)) {
            std::cout << "User registered successfully!" << std::endl;
        } else {
            std::cout << "Registration failed (expected - invalid username)" << std::endl;
        }
    }
    
    std::cout << "\\n=== All tests completed ===" << std::endl;
    
    return 0;
}
