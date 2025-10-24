#include <iostream>
#include <string>
#include <vector>

bool isStrongPassword(const std::string& password) {
    if (password.size() < 8) return false;

    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;

    for (char ch : password) {
        if (!hasLower && std::islower(static_cast<unsigned char>(ch))) hasLower = true;
        else if (!hasUpper && std::isupper(static_cast<unsigned char>(ch))) hasUpper = true;
        else if (!hasDigit && std::isdigit(static_cast<unsigned char>(ch))) hasDigit = true;

        if (hasLower && hasUpper && hasDigit) break;
    }

    return hasLower && hasUpper && hasDigit;
}

int main() {
    std::vector<std::string> tests = {
        "Password1",
        "password",
        "PASSWORD1",
        "Pass1",
        "StrongPass123"
    };

    for (size_t i = 0; i < tests.size(); ++i) {
        bool result = isStrongPassword(tests[i]);
        std::cout << "Test case " << (i + 1) << ": " << (result ? "1" : "0") << std::endl;
    }

    return 0;
}