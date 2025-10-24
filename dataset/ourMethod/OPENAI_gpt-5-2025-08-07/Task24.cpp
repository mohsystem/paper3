#include <iostream>
#include <string>
#include <cctype>

bool validatePIN(const std::string& s) {
    const size_t len = s.size();
    if (!(len == 4 || len == 6)) {
        return false;
    }
    for (char ch : s) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
    }
    return true;
}

int main() {
    // 5 test cases
    std::string tests[] = {"1234", "12345", "a234", "000000", ""};
    for (const auto& t : tests) {
        bool res = validatePIN(t);
        std::cout << "Input: \"" << t << "\" -> " << (res ? "true" : "false") << "\n";
    }
    return 0;
}