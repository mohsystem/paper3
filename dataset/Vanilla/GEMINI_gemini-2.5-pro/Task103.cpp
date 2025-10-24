#include <iostream>
#include <string>
#include <cctype>
#include <vector>

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
    std::vector<std::string> testPasswords = {
        "StrongPassword123",
        "weak",
        "NoDigitsHere",
        "nouppercase123",
        "NOLOWERCASE123"
    };

    for (const auto& password : testPasswords) {
        bool isStrong = checkPasswordStrength(password);
        std::cout << "Password: \"" << password << "\" -> " 
                  << (isStrong ? "Strong" : "Weak") << std::endl;
    }

    return 0;
}