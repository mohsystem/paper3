#include <iostream>
#include <regex>
#include <string>
#include <vector>

bool isValidEmail(const std::string& s) {
    static const std::regex r(R"(^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$)");
    return std::regex_match(s, r);
}

bool isValidPhone(const std::string& s) {
    static const std::regex r(R"(^\+?[0-9]{1,3}?[-. ]?(\([0-9]{3}\)|[0-9]{3})[-. ]?[0-9]{3}[-. ]?[0-9]{4}$)");
    return std::regex_match(s, r);
}

bool isValidUsername(const std::string& s) {
    static const std::regex r(R"(^[A-Za-z][A-Za-z0-9_]{2,15}$)");
    return std::regex_match(s, r);
}

int main() {
    std::vector<std::string> emails = {
        "user@example.com",
        "foo.bar+tag@sub.domain.co",
        "bad@domain",
        "noatsymbol.com",
        "user@domain.c"
    };
    std::vector<std::string> phones = {
        "+1 (555) 123-4567",
        "555-123-4567",
        "5551234567",
        "123-45-6789",
        "++1 555 123 4567"
    };
    std::vector<std::string> usernames = {
        "Alice_123",
        "a",
        "1start",
        "Good_Name",
        "ThisUsernameIsWayTooLong123"
    };

    std::cout << "Email validations:\n";
    for (const auto& e : emails) {
        std::cout << e << " -> " << (isValidEmail(e) ? "true" : "false") << "\n";
    }
    std::cout << "\nPhone validations:\n";
    for (const auto& p : phones) {
        std::cout << p << " -> " << (isValidPhone(p) ? "true" : "false") << "\n";
    }
    std::cout << "\nUsername validations:\n";
    for (const auto& u : usernames) {
        std::cout << u << " -> " << (isValidUsername(u) ? "true" : "false") << "\n";
    }
    return 0;
}