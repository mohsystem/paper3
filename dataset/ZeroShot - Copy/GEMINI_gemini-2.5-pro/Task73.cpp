#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>
#include <unordered_set>

const int MIN_PASSWORD_LENGTH = 12;
const std::unordered_set<std::string> COMMON_PASSWORDS = {
    "password", "123456", "123456789", "qwerty", "12345", "12345678", 
    "111111", "1234567", "password123"
};

/**
 * Checks if a given password is valid for an admin user based on security policies.
 *
 * @param password The password string to validate.
 * @return true if the password is valid, false otherwise.
 */
bool isAdminPasswordValid(const std::string& password) {
    if (password.length() < MIN_PASSWORD_LENGTH) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char c : password) {
        if (isupper(c)) hasUpper = true;
        else if (islower(c)) hasLower = true;
        else if (isdigit(c)) hasDigit = true;
        else if (ispunct(c) || std::string("~`!@#$%^&*()_+-=[]{}|;':\",./<>?").find(c) != std::string::npos) {
            hasSpecial = true;
        }
    }
    
    if (!hasUpper || !hasLower || !hasDigit || !hasSpecial) {
        return false;
    }

    // Check against a list of common passwords
    if (COMMON_PASSWORDS.count(password)) {
        return false;
    }

    // Check if the password contains "admin" (case-insensitive)
    std::string lower_password = password;
    std::transform(lower_password.begin(), lower_password.end(), lower_password.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    
    if (lower_password.find("admin") != std::string::npos) {
        return false;
    }

    return true;
}

int main() {
    std::vector<std::string> testPasswords = {
        "ValidP@ssw0rd!2024",  // Valid
        "Sh0rt!P@s",           // Invalid: Too short
        "NoDigitPassword!",    // Invalid: Missing digit
        "password123",         // Invalid: Common password
        "MyAdm1nPassword!"     // Invalid: Contains "admin"
    };

    std::cout << "Running CPP Test Cases:" << std::endl;
    for (const auto& pwd : testPasswords) {
        std::cout << "Password: \"" << pwd << "\" -> Is Valid: " 
                  << (isAdminPasswordValid(pwd) ? "true" : "false") << std::endl;
    }

    return 0;
}