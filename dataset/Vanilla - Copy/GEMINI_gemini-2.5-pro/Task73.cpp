#include <iostream>
#include <string>
#include <cctype>
#include <algorithm>

class Task73 {
public:
    /**
     * @brief Checks if a given password is valid for an admin user.
     * 
     * A valid admin password must:
     * 1. Be at least 10 characters long.
     * 2. Contain at least one uppercase letter.
     * 3. Contain at least one lowercase letter.
     * 4. Contain at least one digit.
     * 5. Contain at least one special character from the set !@#$%^&*()-_=+[]{}|;:'",.<>/?
     * 6. Contain the substring "admin" (case-insensitive).
     * 
     * @param password The password string to validate.
     * @return true if the password is valid, false otherwise.
     */
    static bool isValidAdminPassword(const std::string& password) {
        if (password.length() < 10) {
            return false;
        }

        bool has_upper = false;
        bool has_lower = false;
        bool has_digit = false;
        bool has_special = false;
        
        const std::string special_chars = "!@#$%^&*()-_=+[]{}|;:'\",.<>/?";

        for (char c : password) {
            if (std::isupper(static_cast<unsigned char>(c))) {
                has_upper = true;
            } else if (std::islower(static_cast<unsigned char>(c))) {
                has_lower = true;
            } else if (std::isdigit(static_cast<unsigned char>(c))) {
                has_digit = true;
            } else if (special_chars.find(c) != std::string::npos) {
                has_special = true;
            }
        }

        // Case-insensitive check for "admin"
        std::string lower_password = password;
        std::transform(lower_password.begin(), lower_password.end(), lower_password.begin(),
                       [](unsigned char c){ return std::tolower(c); });
        
        bool has_admin = (lower_password.find("admin") != std::string::npos);

        return has_upper && has_lower && has_digit && has_special && has_admin;
    }
};

int main() {
    std::string testPasswords[] = {
        "ValidAdminPass123!",   // Should be true
        "shortAd1!",              // Should be false (too short)
        "nouppercaseadmin1!",     // Should be false (no uppercase)
        "NoSpecialAdmin123",      // Should be false (no special character)
        "NoKeyword123!@#",        // Should be false (no "admin" substring)
        "NoDigitsADMIN!@#"        // Should be false (no digits)
    };
    
    std::cout << "Running CPP Test Cases:" << std::endl;
    for (const auto& pass : testPasswords) {
        std::cout << "Password: \"" << pass << "\" is valid? " 
                  << (Task73::isValidAdminPassword(pass) ? "true" : "false") << std::endl;
    }

    return 0;
}