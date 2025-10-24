#include <iostream>
#include <string>
#include <vector>
#include <cctype>

bool isStrongPassword(const std::string& pwd) {
    if (pwd.size() < 8) {
        return false;
    }
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;

    for (char c : pwd) {
        unsigned char uc = static_cast<unsigned char>(c);
        if (std::isupper(uc)) {
            hasUpper = true;
        } else if (std::islower(uc)) {
            hasLower = true;
        } else if (std::isdigit(uc)) {
            hasDigit = true;
        }
        if (hasUpper && hasLower && hasDigit) {
            break;
        }
    }
    return hasUpper && hasLower && hasDigit;
}

int main() {
    std::vector<std::string> tests = {
        "Short7",
        "alllowercase1",
        "ALLUPPERCASE1",
        "NoDigitsHere",
        "Str0ngPass"
    };
    std::vector<bool> expected = {false, false, false, false, true};

    for (size_t i = 0; i < tests.size(); ++i) {
        bool result = isStrongPassword(tests[i]);
        std::cout << "Test " << (i + 1) << ": expected=" << (expected[i] ? "true" : "false")
                  << ", got=" << (result ? "true" : "false")
                  << ", verdict=" << ((result == expected[i]) ? "PASS" : "FAIL") << '\n';
    }
    return 0;
}