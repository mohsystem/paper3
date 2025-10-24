#include <iostream>
#include <string>
#include <unordered_map>
#include <random>
#include <regex>
#include <sstream>
#include <iomanip>

// --- User Data Store ---
struct User {
    std::string username;
    std::string passwordHash; // In a real app, this would be a strong hash (e.g., Argon2, bcrypt)
    std::string resetToken;
};
// In a real application, this would be a database.
std::unordered_map<std::string, User> userStore;

// --- Security Configuration ---
const int MIN_PASSWORD_LENGTH = 12;
// Regex: At least one digit, one lowercase, one uppercase, one special character.
const std::regex PASSWORD_PATTERN("^(?=.*[0-9])(?=.*[a-z])(?=.*[A-Z])(?=.*[@#$%^&+=!])(?=\\S+$).{12,}$");

/**
 * SECURITY: This is a placeholder for a real password hashing function.
 * NEVER store passwords in plaintext or with a simple scheme like this.
 * Use a standard, well-vetted library like OpenSSL's EVP, libsodium, or Argon2.
 */
std::string hashPassword(const std::string& password) {
    return "hashed_" + password;
}

/**
 * Validates if a new password meets the security policy.
 */
bool isPasswordStrong(const std::string& password) {
    return std::regex_match(password, PASSWORD_PATTERN);
}

/**
 * Generates a cryptographically secure random token.
 * Note: std::random_device can be non-deterministic and a good source of entropy on many systems,
 * but for mission-critical security, using OS-specific APIs (like /dev/urandom) or a
 * dedicated crypto library (like OpenSSL) is recommended.
 */
std::string generateResetToken() {
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<unsigned long long> dis;

    std::stringstream ss;
    for (int i = 0; i < 4; ++i) { // 4 * 64 bits = 256 bits
        ss << std::hex << std::setw(16) << std::setfill('0') << dis(gen);
    }
    return ss.str();
}

/**
 * Initiates a password reset request for a user.
 * SECURITY: To prevent user enumeration, this function's observable behavior
 * should be the same whether the user exists or not.
 */
std::string requestPasswordReset(const std::string& username) {
    std::string token = generateResetToken();
    auto it = userStore.find(username);
    if (it != userStore.end()) {
        it->second.resetToken = token;
    }
    return token; // Returned for demonstration; in reality, this would be emailed.
}

/**
 * Resets the user's password if the token is valid and the new password is strong.
 */
bool resetPassword(const std::string& username, const std::string& token, const std::string& newPassword) {
    // 1. Validate new password strength
    if (!isPasswordStrong(newPassword)) {
        std::cout << "Password reset failed for " << username << ": New password is too weak." << std::endl;
        return false;
    }

    auto it = userStore.find(username);

    // 2. Validate user and token
    if (it == userStore.end() || it->second.resetToken.empty() || it->second.resetToken != token) {
        std::cout << "Password reset failed for " << username << ": Invalid username or token." << std::endl;
        return false;
    }

    // 3. Update password and invalidate token
    it->second.passwordHash = hashPassword(newPassword);
    it->second.resetToken = ""; // Invalidate token after use

    std::cout << "Password for " << username << " has been reset successfully." << std::endl;
    return true;
}

int main() {
    // Pre-populate user store
    userStore["alice"] = {"alice", hashPassword("Password123!"), ""};
    userStore["bob"] = {"bob", hashPassword("SecurePass456@"), ""};

    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    // Test Case 1: Successful Password Reset
    std::cout << "\n--- Test Case 1: Successful Reset ---" << std::endl;
    std::string aliceToken = requestPasswordReset("alice");
    std::cout << "Alice requested a reset token: " << aliceToken << std::endl;
    bool success1 = resetPassword("alice", aliceToken, "NewStrongP@ssw0rd123");
    std::cout << "Result: " << (success1 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 2: Failed Reset - Invalid Token
    std::cout << "\n--- Test Case 2: Invalid Token ---" << std::endl;
    std::string bobToken = requestPasswordReset("bob");
    std::cout << "Bob requested a reset token: " << bobToken << std::endl;
    bool success2 = resetPassword("bob", "thisIsAWrongToken12345", "AnotherP@ssword1");
    std::cout << "Result: " << (success2 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 3: Failed Reset - Weak Password
    std::cout << "\n--- Test Case 3: Weak Password ---" << std::endl;
    bool success3 = resetPassword("bob", bobToken, "weak");
    std::cout << "Result: " << (success3 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 4: Failed Reset - Non-existent User
    std::cout << "\n--- Test Case 4: Non-existent User ---" << std::endl;
    std::string charlieToken = requestPasswordReset("charlie");
    std::cout << "A token was generated for a potential user 'charlie': " << charlieToken << std::endl;
    bool success4 = resetPassword("charlie", charlieToken, "StrongP@ssword123");
    std::cout << "Result: " << (success4 ? "SUCCESS" : "FAILURE") << std::endl;

    // Test Case 5: Failed Reset - Token Reuse
    std::cout << "\n--- Test Case 5: Token Reuse ---" << std::endl;
    bool firstReset = resetPassword("bob", bobToken, "BobNewP@ssw0rd987");
    std::cout << "Bob's first reset attempt with valid token: " << (firstReset ? "SUCCESS" : "FAILURE") << std::endl;
    std::cout << "Attempting to reuse Bob's token..." << std::endl;
    bool success5 = resetPassword("bob", bobToken, "SomeOtherP@ssw0rd");
    std::cout << "Result: " << (success5 ? "SUCCESS" : "FAILURE") << std::endl;

    return 0;
}