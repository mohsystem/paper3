
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cctype>

// Password validation function for admin users
// Requirements: min 12 chars, at least one uppercase, lowercase, digit, special char
// No dictionary words or common patterns
bool isValidAdminPassword(const std::string& password) {
    // Input validation: check if password is within acceptable length range
    // Min 12 chars for security, max 128 to prevent DoS attacks
    if (password.empty() || password.length() < 12 || password.length() > 128) {
        return false;
    }
    
    // Security: Prevent null bytes in password which could cause truncation attacks
    if (password.find('\\0') != std::string::npos) {
        return false;
    }
    
    // Check for required character types
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    // Validate each character and check character class requirements
    for (size_t i = 0; i < password.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(password[i]);
        
        // Security: Only allow printable ASCII to prevent encoding attacks
        if (c < 32 || c > 126) {
            return false;
        }
        
        if (std::isupper(c)) hasUpper = true;
        else if (std::islower(c)) hasLower = true;
        else if (std::isdigit(c)) hasDigit = true;
        else if (std::ispunct(c)) hasSpecial = true;
    }
    
    // Require all four character types for admin passwords
    if (!hasUpper || !hasLower || !hasDigit || !hasSpecial) {
        return false;
    }
    
    // Security: Check for common weak patterns
    // Reject sequential characters (e.g., "abc", "123")
    int sequentialCount = 0;
    for (size_t i = 0; i < password.length() - 1; ++i) {
        if (password[i] + 1 == password[i + 1]) {
            sequentialCount++;
            if (sequentialCount >= 3) {
                return false;
            }
        } else {
            sequentialCount = 0;
        }
    }
    
    // Security: Check for repeated characters (e.g., "aaa", "111")
    int repeatCount = 1;
    for (size_t i = 0; i < password.length() - 1; ++i) {
        if (password[i] == password[i + 1]) {
            repeatCount++;
            if (repeatCount >= 3) {
                return false;
            }
        } else {
            repeatCount = 1;
        }
    }
    
    // Security: Reject common weak passwords (case-insensitive check)
    std::string lowerPassword = password;
    std::transform(lowerPassword.begin(), lowerPassword.end(), 
                   lowerPassword.begin(), ::tolower);
    
    const std::vector<std::string> weakPasswords = {
        "password123!", "admin123456!", "administrator1!", 
        "welcome123!", "qwerty123456!"
    };
    
    for (const auto& weak : weakPasswords) {
        if (lowerPassword == weak) {
            return false;
        }
    }
    
    // Security: Check for common dictionary words
    const std::vector<std::string> commonWords = {
        "password", "admin", "administrator", "welcome", 
        "qwerty", "letmein", "monkey", "dragon"
    };
    
    for (const auto& word : commonWords) {
        if (lowerPassword.find(word) != std::string::npos) {
            return false;
        }
    }
    
    return true;
}

int main() {
    // Test case 1: Valid strong password
    std::string test1 = "Adm!n2024Secure#";
    std::cout << "Test 1 - '" << test1 << "': " 
              << (isValidAdminPassword(test1) ? "VALID" : "INVALID") << std::endl;
    
    // Test case 2: Too short (less than 12 characters)
    std::string test2 = "Short1!";
    std::cout << "Test 2 - '" << test2 << "': " 
              << (isValidAdminPassword(test2) ? "VALID" : "INVALID") << std::endl;
    
    // Test case 3: Missing special character
    std::string test3 = "NoSpecialChar123";
    std::cout << "Test 3 - '" << test3 << "': " 
              << (isValidAdminPassword(test3) ? "VALID" : "INVALID") << std::endl;
    
    // Test case 4: Contains common weak pattern
    std::string test4 = "Password123!";
    std::cout << "Test 4 - '" << test4 << "': " 
              << (isValidAdminPassword(test4) ? "VALID" : "INVALID") << std::endl;
    
    // Test case 5: Valid complex password
    std::string test5 = "Xk9$mPz7&Qw3!Rt";
    std::cout << "Test 5 - '" << test5 << "': " 
              << (isValidAdminPassword(test5) ? "VALID" : "INVALID") << std::endl;
    
    return 0;
}
