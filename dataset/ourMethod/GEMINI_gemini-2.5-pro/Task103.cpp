#include <iostream>
#include <string>
#include <cctype>
#include <vector>

/**
 * @brief Checks if a password meets the strength criteria.
 * 
 * Criteria:
 * - At least 8 characters long
 * - Contains at least one uppercase letter
 * - Contains at least one lowercase letter
 * - Contains at least one digit
 * 
 * @param password The password string to check.
 * @return true if the password is strong, false otherwise.
 */
bool isPasswordStrong(const std::string& password) {
    if (password.length() < 8) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (char ch : password) {
        // Cast to unsigned char to prevent undefined behavior with is... functions
        unsigned char u_ch = static_cast<unsigned char>(ch);
        if (std::isupper(u_ch)) {
            hasUpper = true;
        } else if (std::islower(u_ch)) {
            hasLower = true;
        } else if (std::isdigit(u_ch)) {
            hasDigit = true;
        }
        
        // Optimization: if all conditions are met, no need to check further.
        if (hasUpper && hasLower && hasDigit) {
            return true;
        }
    }

    return hasUpper && hasLower && hasDigit;
}

int main() {
    std::vector<std::string> testPasswords = {
        "StrongPwd1",      // Should be true
        "weakpwd1",        // Should be false (missing uppercase)
        "WEAKPWD1",        // Should be false (missing lowercase)
        "WeakPassword",    // Should be false (missing digit)
        "Wpwd1"            // Should be false (too short)
    };
    
    std::cout << std::boolalpha; // Print booleans as "true" or "false"
    for (const auto& password : testPasswords) {
        std::cout << "Password: \"" << password << "\" -> Strong: " 
                  << isPasswordStrong(password) << std::endl;
    }

    return 0;
}