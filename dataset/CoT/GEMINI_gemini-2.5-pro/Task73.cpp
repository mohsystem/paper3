#include <iostream>
#include <string>
#include <cctype>

/**
 * Checks if a given password is valid for an admin user.
 * A valid admin password must:
 * 1. Be at least 12 characters long.
 * 2. Contain at least one uppercase letter.
 * 3. Contain at least one lowercase letter.
 * 4. Contain at least one digit.
 * 5. Contain at least one special character from the set: !@#$%^&*()-_=+[]{}|;:'",.<>/?
 *
 * @param password The password string to validate.
 * @return true if the password is valid, false otherwise.
 */
bool isValidAdminPassword(const std::string& password) {
    // 1. Rule: Check minimum length
    if (password.length() < 12) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    const std::string specialChars = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?";

    for (char c : password) {
        if (isupper(c)) {
            hasUpper = true;
        } else if (islower(c)) {
            hasLower = true;
        } else if (isdigit(c)) {
            hasDigit = true;
        } else if (specialChars.find(c) != std::string::npos) {
            hasSpecial = true;
        }
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}

int main() {
    const char* testPasswords[] = {
        "AdminP@ssw0rd123",  // Valid
        "short",              // Invalid: too short
        "NoDigitPassword!",   // Invalid: no digit
        "nouppercase@123",    // Invalid: no uppercase
        "NoSpecialChar123",   // Invalid: no special character
        nullptr
    };

    std::cout << "Running CPP Test Cases:" << std::endl;
    for (int i = 0; testPasswords[i] != nullptr; ++i) {
        std::string pass(testPasswords[i]);
        std::cout << "Password: \"" << pass << "\" is valid: " 
                  << (isValidAdminPassword(pass) ? "true" : "false") << std::endl;
    }

    return 0;
}