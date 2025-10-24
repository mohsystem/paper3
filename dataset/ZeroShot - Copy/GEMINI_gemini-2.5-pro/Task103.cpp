#include <iostream>
#include <string>
#include <vector>
#include <cctype>

/**
 * @brief Checks the strength of a given password.
 * 
 * A strong password must be at least 8 characters long, contain both
 * uppercase and lowercase letters, and at least one digit.
 * 
 * @param password The password string to validate.
 * @return true if the password is strong, false otherwise.
 */
bool checkPasswordStrength(const std::string& password) {
    if (password.length() < 8) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (char c : password) {
        if (isupper(c)) {
            hasUpper = true;
        } else if (islower(c)) {
            hasLower = true;
        } else if (isdigit(c)) {
            hasDigit = true;
        }
    }

    return hasUpper && hasLower && hasDigit;
}

int main() {
    std::vector<std::string> testCases = {
        "Password123",    // Strong
        "password123",    // Weak (no uppercase)
        "PASSWORD123",    // Weak (no lowercase)
        "Password",       // Weak (no digit)
        "pass"            // Weak (too short)
    };
    
    std::cout << "--- C++ Test Cases ---" << std::endl;
    std::cout << std::boolalpha; // Print "true" or "false" for booleans

    for (const auto& password : testCases) {
        bool isStrong = checkPasswordStrength(password);
        std::cout << "Password: \"" << password << "\" -> Strong: " << isStrong << std::endl;
    }

    return 0;
}