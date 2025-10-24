
#include <iostream>
#include <string>
#include <cctype>

bool checkPasswordStrength(const std::string& password) {
    // Handle empty password securely
    if (password.empty()) {
        return false;
    }
    
    // Check minimum length
    if (password.length() < 8) {
        return false;
    }
    
    bool hasUppercase = false;
    bool hasLowercase = false;
    bool hasDigit = false;
    
    // Iterate through each character to check criteria
    for (size_t i = 0; i < password.length(); i++) {
        char c = password[i];
        
        if (std::isupper(static_cast<unsigned char>(c))) {
            hasUppercase = true;
        } else if (std::islower(static_cast<unsigned char>(c))) {
            hasLowercase = true;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            hasDigit = true;
        }
        
        // Early exit if all criteria met
        if (hasUppercase && hasLowercase && hasDigit) {
            return true;
        }
    }
    
    return hasUppercase && hasLowercase && hasDigit;
}

int main() {
    // Test cases
    std::string testPasswords[] = {
        "Abcd1234",      // Valid - meets all criteria
        "abcd1234",      // Invalid - no uppercase
        "ABCD1234",      // Invalid - no lowercase
        "Abcdefgh",      // Invalid - no digit
        "Abc123"         // Invalid - too short
    };
    
    std::cout << "Password Strength Checker Results:" << std::endl;
    
    for (const auto& password : testPasswords) {
        bool isStrong = checkPasswordStrength(password);
        std::cout << "Password: " << password << " -> " 
                  << (isStrong ? "STRONG" : "WEAK") << std::endl;
    }
    
    return 0;
}
