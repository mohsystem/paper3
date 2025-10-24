#include <iostream>
#include <string>
#include <cctype>

bool isStrongPassword(const std::string& password) {
    if (password.size() < 8) return false;

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (unsigned char ch : password) {
        if (std::isupper(ch)) {
            hasUpper = true;
        } else if (std::islower(ch)) {
            hasLower = true;
        } else if (std::isdigit(ch)) {
            hasDigit = true;
        }
        if (hasUpper && hasLower && hasDigit) {
            return true;
        }
    }
    return hasUpper && hasLower && hasDigit;
}

int main() {
    std::string tests[] = {"Password1", "password1", "PASSWORD1", "Passw1", "Secure123"};
    for (const auto& t : tests) {
        std::cout << t << " -> " << (isStrongPassword(t) ? "true" : "false") << std::endl;
    }
    return 0;
}