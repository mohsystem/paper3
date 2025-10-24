#include <iostream>
#include <string>
#include <cctype>
#include <vector>

/**
 * @brief Checks the strength of a given password.
 * 
 * @param password The password string to check.
 * @return true if the password is strong, false otherwise.
 */
bool checkPasswordStrength(const std::string& password) {
    if (password.length() < 8) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (char ch : password) {
        if (isupper(static_cast<unsigned char>(ch))) {
            hasUpper = true;
        } else if (islower(static_cast<unsigned char>(ch))) {
            hasLower = true;
        } else if (isdigit(static_cast<unsigned char>(ch))) {
            hasDigit = true;
        }
    }

    return hasUpper && hasLower && hasDigit;
}

int main() {
    std::vector<std::string> testCases = {
        "StrongPass123", // Strong
        "weak",          // Weak: too short, no upper, no digit
        "NoDigitPass",   // Weak: no digit
        "nouppercase1",  // Weak: no uppercase
        "NOLOWERCASE1"   // Weak: no lowercase
    };

    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& pass : testCases) {
        std::cout << "Password: \"" << pass << "\" -> Strong: " 
                  << (checkPasswordStrength(pass) ? "true" : "false") << std::endl;
    }

    return 0;
}