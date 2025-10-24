
#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <stdexcept>
#include <iomanip>
#include <sstream>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Securely computes PBKDF2-HMAC-SHA256 hash of password with salt
// Returns hex-encoded string: salt(32 hex chars) + hash(64 hex chars)
std::string hashPasswordWithSalt(const std::string& password, const std::vector<uint8_t>& salt) {
    // Validate inputs - CWE-20: Improper Input Validation
    if (password.empty()) {
        throw std::invalid_argument("Password cannot be empty");
    }
    if (salt.size() != 16) {
        throw std::invalid_argument("Salt must be exactly 16 bytes");
    }
    
    // Use PBKDF2-HMAC-SHA256 with 210000 iterations (OWASP 2023 recommendation)
    // CWE-916: Use of Password Hash With Insufficient Computational Effort
    const int iterations = 210000;
    const int hash_length = 32; // 256 bits
    std::vector<uint8_t> hash(hash_length);
    
    // Derive key using PBKDF2 - CWE-326: Inadequate Encryption Strength
    int result = PKCS5_PBKDF2_HMAC(
        password.c_str(),
        static_cast<int>(password.length()),
        salt.data(),
        static_cast<int>(salt.size()),
        iterations,
        EVP_sha256(),
        hash_length,
        hash.data()
    );
    
    if (result != 1) {
        throw std::runtime_error("PBKDF2 key derivation failed");
    }
    
    // Convert salt and hash to hex string for storage
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    
    // Append salt (16 bytes = 32 hex chars)
    for (size_t i = 0; i < salt.size(); ++i) {
        oss << std::setw(2) << static_cast<int>(salt[i]);
    }
    
    // Append hash (32 bytes = 64 hex chars)
    for (size_t i = 0; i < hash.size(); ++i) {
        oss << std::setw(2) << static_cast<int>(hash[i]);
    }
    
    // Securely clear sensitive data from memory - CWE-244: Improper Clearing of Heap Memory
    OPENSSL_cleanse(hash.data(), hash.size());
    
    return oss.str();
}

// Generate cryptographically secure random salt - CWE-330: Use of Insufficiently Random Values
std::vector<uint8_t> generateSalt() {
    std::vector<uint8_t> salt(16); // CWE-759: Use of a One-Way Hash without a Salt
    
    // Use OpenSSL's cryptographically secure RNG - CWE-338: Use of Cryptographically Weak PRNG\n    int result = RAND_bytes(salt.data(), static_cast<int>(salt.size()));\n    if (result != 1) {\n        throw std::runtime_error("Failed to generate cryptographically secure random salt");\n    }\n    \n    return salt;\n}\n\n// Verify password against stored hash\nbool verifyPassword(const std::string& password, const std::string& storedHash) {\n    // Validate input\n    if (storedHash.length() != 96) { // 32 hex salt + 64 hex hash\n        return false;\n    }\n    \n    // Extract salt from stored hash (first 32 hex chars = 16 bytes)\n    std::vector<uint8_t> salt(16);\n    for (size_t i = 0; i < 16; ++i) {\n        std::string byte_str = storedHash.substr(i * 2, 2);\n        salt[i] = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));\n    }\n    \n    // Compute hash with extracted salt\n    std::string computedHash = hashPasswordWithSalt(password, salt);\n    \n    // Constant-time comparison to prevent timing attacks - CWE-208: Observable Timing Discrepancy\n    bool matches = (computedHash.length() == storedHash.length());\n    for (size_t i = 0; i < computedHash.length() && i < storedHash.length(); ++i) {\n        matches &= (computedHash[i] == storedHash[i]);\n    }\n    \n    return matches && (computedHash.length() == storedHash.length());\n}\n\nint main() {\n    try {\n        // Test case 1: Basic password hashing\n        std::cout << "Test 1: Hash password with generated salt" << std::endl;\n        std::string password1 = "SecurePassword123!";\n        std::vector<uint8_t> salt1 = generateSalt();\n        std::string hash1 = hashPasswordWithSalt(password1, salt1);\n        std::cout << "Hash (salt+hash): " << hash1 << std::endl;\n        std::cout << "Length: " << hash1.length() << " chars (32 salt + 64 hash)" << std::endl;\n        std::cout << std::endl;\n        \n        // Test case 2: Verify password\n        std::cout << "Test 2: Verify correct password" << std::endl;\n        bool valid = verifyPassword(password1, hash1);\n        std::cout << "Verification result: " << (valid ? "PASS" : "FAIL") << std::endl;\n        std::cout << std::endl;\n        \n        // Test case 3: Verify wrong password\n        std::cout << "Test 3: Verify incorrect password" << std::endl;\n        bool invalid = verifyPassword("WrongPassword", hash1);\n        std::cout << "Verification result: " << (invalid ? "FAIL (should be false)" : "PASS (correctly rejected)") << std::endl;\n        std::cout << std::endl;\n        \n        // Test case 4: Same password, different salt produces different hash\n        std::cout << "Test 4: Same password with different salt" << std::endl;\n        std::vector<uint8_t> salt2 = generateSalt();\n        std::string hash2 = hashPasswordWithSalt(password1, salt2);\n        std::cout << "Hash 1: " << hash1.substr(0, 40) << "..." << std::endl;\n        std::cout << "Hash 2: " << hash2.substr(0, 40) << "..." << std::endl;\n        std::cout << "Hashes different: " << (hash1 != hash2 ? "PASS" : "FAIL") << std::endl;\n        std::cout << std::endl;\n        \n        // Test case 5: Complex password\n        std::cout << "Test 5: Complex password with special characters" << std::endl;\n        std::string password3 = "P@ssw0rd!#$%^&*()_+{}[]|\\\\:;\\"'<>,.?/~`";\n        std::vector<uint8_t> salt3 = generateSalt();\n        std::string hash3 = hashPasswordWithSalt(password3, salt3);\n        bool valid3 = verifyPassword(password3, hash3);\n        std::cout << "Hash computed and verified: " << (valid3 ? "PASS" : "FAIL") << std::endl;\n        \n    } catch (const std::exception& e) {\n        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
