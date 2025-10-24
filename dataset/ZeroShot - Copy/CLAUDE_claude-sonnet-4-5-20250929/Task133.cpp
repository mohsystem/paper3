
#include <iostream>
#include <string>
#include <map>
#include <regex>
#include <ctime>
#include <random>
#include <sstream>

class Task133 {
private:
    static const int TOKEN_LENGTH = 32;
    static const int TOKEN_EXPIRY_MINUTES = 15;
    
    struct User {
        std::string email;
        std::string passwordHash;
    };
    
    struct ResetToken {
        std::string token;
        std::string email;
        time_t expiryTime;
        
        bool isExpired() const {
            return time(nullptr) > expiryTime;
        }
    };
    
    std::map<std::string, User> users;
    std::map<std::string, ResetToken> resetTokens;
    
    std::string hashPassword(const std::string& password) {
        return "HASHED_" + password;
    }
    
    bool isValidEmail(const std::string& email) {
        std::regex pattern("^[A-Za-z0-9+_.-]+@[A-Za-z0-9.-]+\\\\.[A-Za-z]{2,}$");
        return std::regex_match(email, pattern);
    }
    
    bool isValidPassword(const std::string& password) {
        if (password.length() < 8) return false;
        
        bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
        std::string specialChars = "@$!%*?&";
        
        for (char c : password) {
            if (isupper(c)) hasUpper = true;
            else if (islower(c)) hasLower = true;
            else if (isdigit(c)) hasDigit = true;
            else if (specialChars.find(c) != std::string::npos) hasSpecial = true;
        }
        
        return hasUpper && hasLower && hasDigit && hasSpecial;
    }
    
    std::string generateRandomToken() {
        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, chars.length() - 1);
        
        std::string token;
        for (int i = 0; i < TOKEN_LENGTH; i++) {
            token += chars[dis(gen)];
        }
        return token;
    }
    
public:
    std::string registerUser(const std::string& email, const std::string& password) {
        if (!isValidEmail(email)) {
            return "Error: Invalid email format";
        }
        
        if (users.find(email) != users.end()) {
            return "Error: User already exists";
        }
        
        User user;
        user.email = email;
        user.passwordHash = hashPassword(password);
        users[email] = user;
        
        return "Success: User registered";
    }
    
    std::string generateResetToken(const std::string& email) {
        if (email.empty() || !isValidEmail(email)) {
            return "Error: Invalid email format";
        }
        
        if (users.find(email) == users.end()) {
            return "Error: Email not found";
        }
        
        std::string token = generateRandomToken();
        time_t expiryTime = time(nullptr) + (TOKEN_EXPIRY_MINUTES * 60);
        
        ResetToken resetToken;
        resetToken.token = token;
        resetToken.email = email;
        resetToken.expiryTime = expiryTime;
        
        resetTokens[token] = resetToken;
        
        return token;
    }
    
    std::string resetPassword(const std::string& token, const std::string& newPassword) {
        if (token.empty()) {
            return "Error: Token is required";
        }
        
        if (newPassword.empty() || !isValidPassword(newPassword)) {
            return "Error: Password must be at least 8 characters with uppercase, lowercase, digit, and special character";
        }
        
        auto it = resetTokens.find(token);
        if (it == resetTokens.end()) {
            return "Error: Invalid token";
        }
        
        if (it->second.isExpired()) {
            resetTokens.erase(it);
            return "Error: Token has expired";
        }
        
        std::string email = it->second.email;
        users[email].passwordHash = hashPassword(newPassword);
        resetTokens.erase(it);
        
        return "Success: Password reset successfully";
    }
};

int main() {
    std::cout << "=== Password Reset Functionality Tests ===\\n\\n";
    
    Task133 system;
    
    // Test Case 1: Register users
    std::cout << "Test 1: Register users\\n";
    std::cout << system.registerUser("user1@example.com", "Password123!") << "\\n";
    std::cout << system.registerUser("user2@example.com", "Secure456@") << "\\n\\n";
    
    // Test Case 2: Generate reset token for valid user
    std::cout << "Test 2: Generate reset token for valid user\\n";
    std::string token1 = system.generateResetToken("user1@example.com");
    std::cout << "Token generated: " << (token1.find("Error") == 0 ? token1 : "Success (Length: " + std::to_string(token1.length()) + ")") << "\\n\\n";
    
    // Test Case 3: Generate reset token for invalid email
    std::cout << "Test 3: Generate reset token for invalid email\\n";
    std::cout << system.generateResetToken("invalidemail") << "\\n\\n";
    
    // Test Case 4: Reset password with valid token and valid password
    std::cout << "Test 4: Reset password with valid token\\n";
    std::string token2 = system.generateResetToken("user2@example.com");
    if (token2.find("Error") != 0) {
        std::cout << system.resetPassword(token2, "NewPass789#") << "\\n";
    }
    std::cout << "\\n";
    
    // Test Case 5: Reset password with invalid token
    std::cout << "Test 5: Reset password with invalid token\\n";
    std::cout << system.resetPassword("invalidtoken123", "ValidPass123!") << "\\n\\n";
    
    // Test Case 6: Reset password with weak password
    std::cout << "Test 6: Reset password with weak password\\n";
    std::string token3 = system.generateResetToken("user1@example.com");
    if (token3.find("Error") != 0) {
        std::cout << system.resetPassword(token3, "weak") << "\\n";
    }
    
    return 0;
}
