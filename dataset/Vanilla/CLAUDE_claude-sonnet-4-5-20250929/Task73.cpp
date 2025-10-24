
#include <iostream>
#include <string>
#include <cctype>

bool isValidPassword(const std::string& password) {
    if (password.empty() || password.length() < 8) {
        return false;
    }
    
    bool hasUpperCase = false;
    bool hasLowerCase = false;
    bool hasDigit = false;
    bool hasSpecialChar = false;
    
    std::string specialChars = "!@#$%^&*()_+-=[]{}|;:,.<>?";
    
    for (char c : password) {
        if (std::isupper(c)) {
            hasUpperCase = true;
        } else if (std::islower(c)) {
            hasLowerCase = true;
        } else if (std::isdigit(c)) {
            hasDigit = true;
        } else if (specialChars.find(c) != std::string::npos) {
            hasSpecialChar = true;
        }
    }
    
    return hasUpperCase && hasLowerCase && hasDigit && hasSpecialChar;
}

int main() {
    // Test cases
    std::cout << "Test 1: 'Admin@123' -> " << (isValidPassword("Admin@123") ? "true" : "false") << std::endl;
    std::cout << "Test 2: 'password' -> " << (isValidPassword("password") ? "true" : "false") << std::endl;
    std::cout << "Test 3: 'Pass123' -> " << (isValidPassword("Pass123") ? "true" : "false") << std::endl;
    std::cout << "Test 4: 'ADMIN@123' -> " << (isValidPassword("ADMIN@123") ? "true" : "false") << std::endl;
    std::cout << "Test 5: 'Secure#Pass2024' -> " << (isValidPassword("Secure#Pass2024") ? "true" : "false") << std::endl;
    
    return 0;
}
