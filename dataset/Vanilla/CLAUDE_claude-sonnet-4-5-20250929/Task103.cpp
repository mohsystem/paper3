
#include <iostream>
#include <string>
#include <cctype>

std::string checkPasswordStrength(const std::string& password) {
    if (password.empty()) {
        return "Weak";
    }
    
    bool hasMinLength = password.length() >= 8;
    bool hasUppercase = false;
    bool hasLowercase = false;
    bool hasDigit = false;
    
    for (char c : password) {
        if (std::isupper(c)) {
            hasUppercase = true;
        } else if (std::islower(c)) {
            hasLowercase = true;
        } else if (std::isdigit(c)) {
            hasDigit = true;
        }
    }
    
    if (hasMinLength && hasUppercase && hasLowercase && hasDigit) {
        return "Strong";
    } else {
        return "Weak";
    }
}

int main() {
    // Test case 1: Strong password
    std::cout << "Test 1 - 'Password123': " << checkPasswordStrength("Password123") << std::endl;
    
    // Test case 2: Too short
    std::cout << "Test 2 - 'Pass1': " << checkPasswordStrength("Pass1") << std::endl;
    
    // Test case 3: No uppercase
    std::cout << "Test 3 - 'password123': " << checkPasswordStrength("password123") << std::endl;
    
    // Test case 4: No digit
    std::cout << "Test 4 - 'PasswordTest': " << checkPasswordStrength("PasswordTest") << std::endl;
    
    // Test case 5: Strong password with special characters
    std::cout << "Test 5 - 'SecurePass99!': " << checkPasswordStrength("SecurePass99!") << std::endl;
    
    return 0;
}
