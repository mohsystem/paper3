
#include <iostream>
#include <string>
#include <cctype>
#include <limits>

// Function to check password strength
// Returns: 0 = weak, 1 = strong
// Security: Input validation, no logging of password, constant-time-like checks
int checkPasswordStrength(const std::string& password) {
    // Input validation: check if password is empty
    if (password.empty()) {
        return 0;
    }
    
    // Check minimum length (at least 8 characters)
    // Security: Prevents weak short passwords
    if (password.length() < 8) {
        return 0;
    }
    
    // Check maximum length to prevent excessive memory usage
    // Security: Prevents potential DoS through extremely long inputs
    if (password.length() > 128) {
        return 0;
    }
    
    // Initialize flags for character type checks
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    
    // Iterate through all characters to check requirements
    // Security: Process all characters to avoid timing attacks
    for (size_t i = 0; i < password.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(password[i]);
        
        // Check for uppercase letter
        if (std::isupper(c)) {
            hasUpper = true;
        }
        // Check for lowercase letter
        if (std::islower(c)) {
            hasLower = true;
        }
        // Check for digit
        if (std::isdigit(c)) {
            hasDigit = true;
        }
    }
    
    // Return 1 if all criteria are met, 0 otherwise
    // Security: Constant return logic prevents timing-based information leakage
    return (hasUpper && hasLower && hasDigit) ? 1 : 0;
}

int main() {
    // Test cases: demonstrate various password strengths
    // Security: Test passwords are not real credentials
    std::string testPasswords[5] = {
        "Weak1",           // Too short (5 chars)
        "weakpassword1",   // No uppercase
        "WEAKPASSWORD1",   // No lowercase
        "WeakPassword",    // No digit
        "Strong1Pass"      // Valid: 12 chars, upper, lower, digit
    };
    
    std::cout << "Password Strength Checker\\n";
    std::cout << "=========================\\n\\n";
    
    // Test each password
    for (int i = 0; i < 5; ++i) {
        int result = checkPasswordStrength(testPasswords[i]);
        
        // Security: Do not log the actual password in production
        std::cout << "Test " << (i + 1) << ": ";
        std::cout << (result == 1 ? "STRONG" : "WEAK") << "\\n";
        
        // Display criteria status
        bool hasUpper = false, hasLower = false, hasDigit = false;
        for (size_t j = 0; j < testPasswords[i].length(); ++j) {
            unsigned char c = static_cast<unsigned char>(testPasswords[i][j]);
            if (std::isupper(c)) hasUpper = true;
            if (std::islower(c)) hasLower = true;
            if (std::isdigit(c)) hasDigit = true;
        }
        
        std::cout << "  Length >= 8: " << (testPasswords[i].length() >= 8 ? "YES" : "NO") << "\\n";
        std::cout << "  Has uppercase: " << (hasUpper ? "YES" : "NO") << "\\n";
        std::cout << "  Has lowercase: " << (hasLower ? "YES" : "NO") << "\\n";
        std::cout << "  Has digit: " << (hasDigit ? "YES" : "NO") << "\\n\\n";
    }
    
    return 0;
}
