
#include <iostream>
#include <string>
#include <cstring>
#include <random>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <memory>
#include <limits>

// Use OpenSSL for cryptographic operations
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Constants for security
constexpr size_t OTP_LENGTH = 6;
constexpr size_t SALT_SIZE = 16;
constexpr int PBKDF2_ITERATIONS = 210000;
constexpr size_t HASH_SIZE = 32;
constexpr int OTP_VALIDITY_SECONDS = 300; // 5 minutes

// Structure to hold user credentials securely
struct UserCredentials {
    std::string username;
    unsigned char password_hash[HASH_SIZE];
    unsigned char salt[SALT_SIZE];
    
    UserCredentials() {
        // Initialize arrays to zero
        std::memset(password_hash, 0, HASH_SIZE);
        std::memset(salt, 0, SALT_SIZE);
    }
    
    ~UserCredentials() {
        // Securely clear sensitive data
        OPENSSL_cleanse(password_hash, HASH_SIZE);
        OPENSSL_cleanse(salt, SALT_SIZE);
    }
};

// Structure to hold OTP data
struct OTPData {
    std::string otp;
    std::chrono::system_clock::time_point created_at;
    
    OTPData() : otp(""), created_at(std::chrono::system_clock::now()) {}
};

// Generate cryptographically secure random OTP
// Returns empty string on failure
std::string generateSecureOTP() {
    unsigned char random_bytes[OTP_LENGTH];
    
    // Use OpenSSL's cryptographically secure RNG\n    if (RAND_bytes(random_bytes, OTP_LENGTH) != 1) {\n        std::cerr << "Error: Failed to generate secure random bytes" << std::endl;\n        return "";\n    }\n    \n    std::string otp;\n    otp.reserve(OTP_LENGTH);\n    \n    // Convert to numeric digits (0-9)\n    for (size_t i = 0; i < OTP_LENGTH; ++i) {\n        // Modulo 10 to get digit, no bias significant for OTP use case\n        otp += std::to_string(random_bytes[i] % 10);\n    }\n    \n    // Securely clear random bytes\n    OPENSSL_cleanse(random_bytes, OTP_LENGTH);\n    \n    return otp;\n}\n\n// Derive password hash using PBKDF2-HMAC-SHA256\n// Returns true on success\nbool derivePasswordHash(const std::string& password, const unsigned char* salt, \n                       unsigned char* out_hash) {\n    if (password.empty() || !salt || !out_hash) {\n        return false;\n    }\n    \n    // Validate password length to prevent excessive memory usage\n    if (password.length() > 128) {\n        std::cerr << "Error: Password too long" << std::endl;\n        return false;\n    }\n    \n    // Use PBKDF2-HMAC-SHA256 with 210000 iterations\n    int result = PKCS5_PBKDF2_HMAC(\n        password.c_str(), \n        static_cast<int>(password.length()),\n        salt, \n        SALT_SIZE,\n        PBKDF2_ITERATIONS,\n        EVP_sha256(),\n        HASH_SIZE,\n        out_hash\n    );\n    \n    return result == 1;\n}\n\n// Generate unique salt for each user\nbool generateSalt(unsigned char* salt) {\n    if (!salt) {\n        return false;\n    }\n    \n    // Use OpenSSL's cryptographically secure RNG
    return RAND_bytes(salt, SALT_SIZE) == 1;
}

// Register a new user with secure password hashing
bool registerUser(const std::string& username, const std::string& password, 
                 UserCredentials& creds) {
    // Validate inputs - treat all inputs as untrusted
    if (username.empty() || username.length() > 64) {
        std::cerr << "Error: Invalid username length" << std::endl;
        return false;
    }
    
    if (password.empty() || password.length() < 8 || password.length() > 128) {
        std::cerr << "Error: Password must be 8-128 characters" << std::endl;
        return false;
    }
    
    // Generate unique salt
    if (!generateSalt(creds.salt)) {
        std::cerr << "Error: Failed to generate salt" << std::endl;
        return false;
    }
    
    // Derive password hash with salt
    if (!derivePasswordHash(password, creds.salt, creds.password_hash)) {
        std::cerr << "Error: Failed to hash password" << std::endl;
        OPENSSL_cleanse(creds.salt, SALT_SIZE);
        return false;
    }
    
    creds.username = username;
    return true;
}

// Verify password using constant-time comparison
bool verifyPassword(const std::string& password, const UserCredentials& creds) {
    if (password.empty()) {
        return false;
    }
    
    unsigned char computed_hash[HASH_SIZE];
    std::memset(computed_hash, 0, HASH_SIZE);
    
    // Derive hash from provided password
    if (!derivePasswordHash(password, creds.salt, computed_hash)) {
        OPENSSL_cleanse(computed_hash, HASH_SIZE);
        return false;
    }
    
    // Use constant-time comparison to prevent timing attacks
    int result = CRYPTO_memcmp(computed_hash, creds.password_hash, HASH_SIZE);
    
    // Securely clear computed hash
    OPENSSL_cleanse(computed_hash, HASH_SIZE);
    
    return result == 0;
}

// Verify OTP with timing check
bool verifyOTP(const std::string& provided_otp, const OTPData& otp_data) {
    // Check if OTP has expired
    auto now = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - otp_data.created_at);
    
    if (elapsed.count() > OTP_VALIDITY_SECONDS) {
        std::cerr << "Error: OTP expired" << std::endl;
        return false;
    }
    
    // Validate input format
    if (provided_otp.length() != OTP_LENGTH) {
        return false;
    }
    
    // Check all characters are digits
    for (char c : provided_otp) {
        if (c < '0' || c > '9') {
            return false;
        }
    }
    
    // Use constant-time comparison
    if (provided_otp.length() != otp_data.otp.length()) {
        return false;
    }
    
    int result = CRYPTO_memcmp(provided_otp.c_str(), otp_data.otp.c_str(), OTP_LENGTH);
    return result == 0;
}

// Two-factor authentication login
bool twoFactorLogin(const std::string& username, const std::string& password,
                   const std::string& otp_input, const UserCredentials& creds,
                   const OTPData& otp_data) {
    // Step 1: Verify username
    if (username != creds.username) {
        std::cerr << "Error: Invalid username" << std::endl;
        return false;
    }
    
    // Step 2: Verify password
    if (!verifyPassword(password, creds)) {
        std::cerr << "Error: Invalid password" << std::endl;
        return false;
    }
    
    // Step 3: Verify OTP
    if (!verifyOTP(otp_input, otp_data)) {
        std::cerr << "Error: Invalid or expired OTP" << std::endl;
        return false;
    }
    
    return true;
}

// Safely read input with length validation
std::string readInput(const std::string& prompt, size_t max_length) {
    std::cout << prompt;
    std::string input;
    
    if (!std::getline(std::cin, input)) {
        return "";
    }
    
    // Validate length
    if (input.length() > max_length) {
        std::cerr << "Error: Input too long" << std::endl;
        return "";
    }
    
    return input;
}

int main() {
    // Initialize OpenSSL
    OpenSSL_add_all_algorithms();
    
    std::cout << "=== Two-Factor Authentication System ===" << std::endl;
    
    // Test Case 1: Successful registration and login
    std::cout << "\\nTest Case 1: Successful 2FA flow" << std::endl;
    {
        UserCredentials user;
        if (!registerUser("alice", "SecurePass123!", user)) {
            std::cerr << "Registration failed" << std::endl;
            return 1;
        }
        
        OTPData otp_data;
        otp_data.otp = generateSecureOTP();
        if (otp_data.otp.empty()) {
            std::cerr << "OTP generation failed" << std::endl;
            return 1;
        }
        
        std::cout << "User registered: alice" << std::endl;
        std::cout << "Generated OTP: " << otp_data.otp << " (valid for " 
                  << OTP_VALIDITY_SECONDS << " seconds)" << std::endl;
        
        if (twoFactorLogin("alice", "SecurePass123!", otp_data.otp, user, otp_data)) {
            std::cout << "✓ Login successful!" << std::endl;
        } else {
            std::cout << "✗ Login failed" << std::endl;
        }
    }
    
    // Test Case 2: Wrong password
    std::cout << "\\nTest Case 2: Wrong password" << std::endl;
    {
        UserCredentials user;
        registerUser("bob", "MyPassword456", user);
        
        OTPData otp_data;
        otp_data.otp = generateSecureOTP();
        
        std::cout << "Generated OTP: " << otp_data.otp << std::endl;
        
        if (twoFactorLogin("bob", "WrongPassword", otp_data.otp, user, otp_data)) {
            std::cout << "✓ Login successful!" << std::endl;
        } else {
            std::cout << "✗ Login failed (expected)" << std::endl;
        }
    }
    
    // Test Case 3: Wrong OTP
    std::cout << "\\nTest Case 3: Wrong OTP" << std::endl;
    {
        UserCredentials user;
        registerUser("charlie", "StrongPass789", user);
        
        OTPData otp_data;
        otp_data.otp = generateSecureOTP();
        
        std::cout << "Generated OTP: " << otp_data.otp << std::endl;
        
        if (twoFactorLogin("charlie", "StrongPass789", "000000", user, otp_data)) {
            std::cout << "✓ Login successful!" << std::endl;
        } else {
            std::cout << "✗ Login failed (expected)" << std::endl;
        }
    }
    
    // Test Case 4: Expired OTP
    std::cout << "\\nTest Case 4: Expired OTP" << std::endl;
    {
        UserCredentials user;
        registerUser("david", "Password2024", user);
        
        OTPData otp_data;
        otp_data.otp = generateSecureOTP();
        // Simulate expired OTP by setting old timestamp
        otp_data.created_at = std::chrono::system_clock::now() - 
                             std::chrono::seconds(OTP_VALIDITY_SECONDS + 1);
        
        std::cout << "Generated OTP: " << otp_data.otp << " (expired)" << std::endl;
        
        if (twoFactorLogin("david", "Password2024", otp_data.otp, user, otp_data)) {
            std::cout << "✓ Login successful!" << std::endl;
        } else {
            std::cout << "✗ Login failed (expected)" << std::endl;
        }
    }
    
    // Test Case 5: Multiple OTP generations
    std::cout << "\\nTest Case 5: Multiple OTP generations" << std::endl;
    {
        UserCredentials user;
        registerUser("eve", "ComplexPass!23", user);
        
        std::cout << "Generating 3 unique OTPs:" << std::endl;
        for (int i = 0; i < 3; ++i) {
            OTPData otp_data;
            otp_data.otp = generateSecureOTP();
            std::cout << "  OTP " << (i + 1) << ": " << otp_data.otp << std::endl;
            
            // Verify each OTP works
            if (twoFactorLogin("eve", "ComplexPass!23", otp_data.otp, user, otp_data)) {
                std::cout << "  ✓ OTP " << (i + 1) << " verified" << std::endl;
            }
        }
    }
    
    // Cleanup OpenSSL
    EVP_cleanup();
    
    return 0;
}
