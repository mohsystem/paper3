
#include <iostream>
#include <string>
#include <cstring>
#include <random>
#include <iomanip>
#include <sstream>
#include <vector>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

// Password reset functionality with secure token generation and validation
// Uses PBKDF2-HMAC-SHA256 for password hashing with unique salts

class PasswordReset {
private:
    static constexpr int SALT_SIZE = 16;
    static constexpr int HASH_SIZE = 32;
    static constexpr int TOKEN_SIZE = 32;
    static constexpr int PBKDF2_ITERATIONS = 210000;
    static constexpr int TOKEN_EXPIRY_SECONDS = 3600; // 1 hour

    // Secure comparison to prevent timing attacks
    static bool constantTimeCompare(const std::vector<unsigned char>& a, 
                                   const std::vector<unsigned char>& b) {
        if (a.size() != b.size()) {
            return false;
        }
        unsigned char result = 0;
        for (size_t i = 0; i < a.size(); i++) {
            result |= a[i] ^ b[i];
        }
        return result == 0;
    }

    // Generate cryptographically secure random bytes
    static bool generateRandomBytes(unsigned char* buffer, int size) {
        if (!buffer || size <= 0) {
            return false;
        }
        // Use OpenSSL's CSPRNG\n        if (RAND_bytes(buffer, size) != 1) {\n            return false;\n        }\n        return true;\n    }\n\n    // Hash password with PBKDF2-HMAC-SHA256\n    static bool hashPassword(const std::string& password, \n                            const unsigned char* salt,\n                            unsigned char* hash) {\n        if (password.empty() || !salt || !hash) {\n            return false;\n        }\n\n        // Use PBKDF2 with SHA-256 and 210,000 iterations\n        if (PKCS5_PBKDF2_HMAC(password.c_str(), password.length(),\n                              salt, SALT_SIZE,\n                              PBKDF2_ITERATIONS,\n                              EVP_sha256(),\n                              HASH_SIZE, hash) != 1) {\n            return false;\n        }\n        return true;\n    }\n\npublic:\n    struct UserRecord {\n        std::string username;\n        std::vector<unsigned char> passwordHash;\n        std::vector<unsigned char> salt;\n        std::vector<unsigned char> resetToken;\n        long tokenExpiry; // Timestamp\n    };\n\n    // Generate a secure password reset token\n    static bool generateResetToken(UserRecord& user) {\n        if (user.username.empty()) {\n            return false;\n        }\n\n        // Generate cryptographically secure random token\n        user.resetToken.resize(TOKEN_SIZE);\n        if (!generateRandomBytes(user.resetToken.data(), TOKEN_SIZE)) {\n            user.resetToken.clear();\n            return false;\n        }\n\n        // Set token expiry (current time + 1 hour)\n        user.tokenExpiry = static_cast<long>(time(nullptr)) + TOKEN_EXPIRY_SECONDS;\n        return true;\n    }\n\n    // Validate reset token\n    static bool validateResetToken(const UserRecord& user, \n                                  const std::vector<unsigned char>& token) {\n        if (user.resetToken.empty() || token.empty()) {\n            return false;\n        }\n\n        // Check token expiry\n        long currentTime = static_cast<long>(time(nullptr));\n        if (currentTime > user.tokenExpiry) {\n            return false;\n        }\n\n        // Constant-time comparison to prevent timing attacks\n        return constantTimeCompare(user.resetToken, token);\n    }\n\n    // Hash and store new password\n    static bool resetPassword(UserRecord& user, const std::string& newPassword) {\n        // Validate password strength (minimum 8 characters, must contain variety)\n        if (newPassword.length() < 8) {\n            return false;\n        }\n\n        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;\n        for (char c : newPassword) {\n            if (isupper(c)) hasUpper = true;\n            else if (islower(c)) hasLower = true;\n            else if (isdigit(c)) hasDigit = true;\n            else hasSpecial = true;\n        }\n\n        if (!hasUpper || !hasLower || !hasDigit || !hasSpecial) {\n            return false;\n        }\n\n        // Generate new unique salt\n        user.salt.resize(SALT_SIZE);\n        if (!generateRandomBytes(user.salt.data(), SALT_SIZE)) {\n            user.salt.clear();\n            return false;\n        }\n\n        // Hash password with new salt\n        user.passwordHash.resize(HASH_SIZE);\n        if (!hashPassword(newPassword, user.salt.data(), user.passwordHash.data())) {\n            // Clear sensitive data on failure\n            user.passwordHash.clear();\n            user.salt.clear();\n            return false;\n        }\n\n        // Clear reset token after successful password reset\n        user.resetToken.clear();\n        user.tokenExpiry = 0;\n\n        return true;\n    }\n\n    // Verify password against stored hash\n    static bool verifyPassword(const UserRecord& user, const std::string& password) {\n        if (user.passwordHash.empty() || user.salt.empty() || password.empty()) {\n            return false;\n        }\n\n        std::vector<unsigned char> computedHash(HASH_SIZE);\n        if (!hashPassword(password, user.salt.data(), computedHash.data())) {\n            return false;\n        }\n\n        return constantTimeCompare(user.passwordHash, computedHash);\n    }\n\n    // Convert bytes to hex string for display (not for secrets)\n    static std::string toHexString(const std::vector<unsigned char>& data) {\n        std::stringstream ss;\n        for (unsigned char byte : data) {\n            ss << std::hex << std::setw(2) << std::setfill('0') 
               << static_cast<int>(byte);
        }
        return ss.str();
    }

    // Convert hex string to bytes
    static std::vector<unsigned char> fromHexString(const std::string& hex) {
        std::vector<unsigned char> bytes;
        if (hex.length() % 2 != 0) {
            return bytes;
        }
        
        for (size_t i = 0; i < hex.length(); i += 2) {
            std::string byteString = hex.substr(i, 2);
            unsigned char byte = static_cast<unsigned char>(
                std::strtol(byteString.c_str(), nullptr, 16));
            bytes.push_back(byte);
        }
        return bytes;
    }
};

int main() {
    std::cout << "=== Password Reset Functionality Test Cases ===" << std::endl;

    // Test Case 1: Create user and set initial password
    std::cout << "\\nTest 1: Create user with initial password" << std::endl;
    PasswordReset::UserRecord user;
    user.username = "testuser1";
    if (PasswordReset::resetPassword(user, "SecureP@ss123")) {
        std::cout << "Initial password set successfully" << std::endl;
    } else {
        std::cout << "Failed to set initial password" << std::endl;
    }

    // Test Case 2: Generate reset token
    std::cout << "\\nTest 2: Generate password reset token" << std::endl;
    if (PasswordReset::generateResetToken(user)) {
        std::cout << "Reset token generated (length: " 
                  << user.resetToken.size() << " bytes)" << std::endl;
    } else {
        std::cout << "Failed to generate reset token" << std::endl;
    }

    // Test Case 3: Validate correct reset token
    std::cout << "\\nTest 3: Validate correct reset token" << std::endl;
    std::vector<unsigned char> validToken = user.resetToken;
    if (PasswordReset::validateResetToken(user, validToken)) {
        std::cout << "Token validation successful" << std::endl;
    } else {
        std::cout << "Token validation failed" << std::endl;
    }

    // Test Case 4: Validate incorrect reset token
    std::cout << "\\nTest 4: Validate incorrect reset token" << std::endl;
    std::vector<unsigned char> invalidToken(32, 0xFF);
    if (!PasswordReset::validateResetToken(user, invalidToken)) {
        std::cout << "Invalid token correctly rejected" << std::endl;
    } else {
        std::cout << "Invalid token incorrectly accepted" << std::endl;
    }

    // Test Case 5: Reset password with valid token and verify
    std::cout << "\\nTest 5: Reset password and verify" << std::endl;
    if (PasswordReset::resetPassword(user, "NewSecure@Pass456")) {
        std::cout << "Password reset successfully" << std::endl;
        if (PasswordReset::verifyPassword(user, "NewSecure@Pass456")) {
            std::cout << "New password verified successfully" << std::endl;
        } else {
            std::cout << "New password verification failed" << std::endl;
        }
        if (!PasswordReset::verifyPassword(user, "SecureP@ss123")) {
            std::cout << "Old password correctly rejected" << std::endl;
        } else {
            std::cout << "Old password incorrectly accepted" << std::endl;
        }
    } else {
        std::cout << "Failed to reset password" << std::endl;
    }

    return 0;
}
