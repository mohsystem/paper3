
#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>
#include <unordered_set>

bool isValidAdminPassword(const std::string& password) {
    const int MIN_PASSWORD_LENGTH = 12;
    static const std::unordered_set<std::string> COMMON_PASSWORDS = {
        "password123!", "admin@123456", "welcome@2024", 
        "p@ssw0rd1234", "administrator1!"
    };
    
    // Check for empty password
    if (password.empty()) {
        return false;
    }
    
    // Check minimum length
    if (password.length() < MIN_PASSWORD_LENGTH) {
        return false;
    }
    
    // Check for common passwords (case-insensitive)
    std::string lowerPassword = password;
    std::transform(lowerPassword.begin(), lowerPassword.end(), 
                   lowerPassword.begin(), ::tolower);
    
    if (COMMON_PASSWORDS.find(lowerPassword) != COMMON_PASSWORDS.end()) {
        return false;
    }
    
    // Check for required character types
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    for (char c : password) {
        if (std::isupper(c)) {
            hasUpper = true;
        } else if (std::islower(c)) {
            hasLower = true;
        } else if (std::isdigit(c)) {
            hasDigit = true;
        } else if (!std::isspace(c)) {
            hasSpecial = true;
        }
    }
    
    return hasUpper && hasLower && hasDigit && hasSpecial;
}

int main() {
    // Test cases
    std::string testPasswords[] = {
        "SecureP@ssw0rd123",  // Valid
        "Weak1!",              // Too short
        "nosymbolsupper1",     // No uppercase or special
        "NOLOWERCASE123!",     // No lowercase
        "Admin@123456"         // Common password
    };
    
    std::cout << "Admin Password Validation Results:" << std::endl;
    for (int i = 0; i < 5; i++) {
        bool result = isValidAdminPassword(testPasswords[i]);
        std::cout << "Test " << (i + 1) << ": \\"" << testPasswords[i] 
                  << "\\" -> " << (result ? "VALID" : "INVALID") << std::endl;
    }
    
    return 0;
}
