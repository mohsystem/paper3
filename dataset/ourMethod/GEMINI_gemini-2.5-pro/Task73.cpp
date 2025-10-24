#include <iostream>
#include <string>
#include <cctype>

const int MIN_LENGTH = 12;
const int MAX_LENGTH = 128;
const std::string SPECIAL_CHARACTERS = "!@#$%^&*()_+-=[]{};':\"\\|,.<>/?~";

/**
 * @brief Checks if a given password is valid for an admin user.
 * 
 * Policy:
 * - Length between 12 and 128 characters.
 * - Contains at least one uppercase letter.
 * - Contains at least one lowercase letter.
 * - Contains at least one digit.
 * - Contains at least one special character.
 * 
 * @param password The password string to validate.
 * @return true if the password is valid, false otherwise.
 */
bool isAdminPasswordValid(const std::string& password) {
    if (password.length() < MIN_LENGTH || password.length() > MAX_LENGTH) {
        return false;
    }

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char ch : password) {
        if (std::isupper(static_cast<unsigned char>(ch))) {
            hasUpper = true;
        } else if (std::islower(static_cast<unsigned char>(ch))) {
            hasLower = true;
        } else if (std::isdigit(static_cast<unsigned char>(ch))) {
            hasDigit = true;
        } else if (SPECIAL_CHARACTERS.find(ch) != std::string::npos) {
            hasSpecial = true;
        }
        
        // Optimization: if all conditions are met, we can exit early.
        if (hasUpper && hasLower && hasDigit && hasSpecial) {
            return true;
        }
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}

int main() {
    const std::string testPasswords[] = {
        "Admin@1234567",       // Valid
        "admin@1234567",       // Invalid: no uppercase
        "Admin1234567",        // Invalid: no special character
        "Admin@",              // Invalid: too short
        "ADMIN@1234567"        // Invalid: no lowercase
    };

    for (const auto& password : testPasswords) {
        std::cout << "Password: \"" << password << "\" is valid: " 
                  << (isAdminPasswordValid(password) ? "true" : "false") << std::endl;
    }

    return 0;
}