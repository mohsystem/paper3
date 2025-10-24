#include <iostream>
#include <string>
#include <cctype>
#include <vector>

bool isStrongPassword(const std::string& password) {
    if (password.size() < 8) return false;
    bool hasUpper = false, hasLower = false, hasDigit = false;
    for (char c : password) {
        if (std::isupper(static_cast<unsigned char>(c))) hasUpper = true;
        else if (std::islower(static_cast<unsigned char>(c))) hasLower = true;
        else if (std::isdigit(static_cast<unsigned char>(c))) hasDigit = true;
    }
    return hasUpper && hasLower && hasDigit;
}

int main() {
    std::vector<std::string> tests = {
        "Password1",
        "password",
        "PASSWORD1",
        "Passw1",
        "StrongPass123"
    };
    for (const auto& t : tests) {
        std::cout << t << " -> " << (isStrongPassword(t) ? "true" : "false") << "\n";
    }
    return 0;
}