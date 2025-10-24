#include <iostream>
#include <regex>
#include <string>
#include <vector>

static bool withinLength(const std::string& s, size_t minLen, size_t maxLen) {
    return s.size() >= minLen && s.size() <= maxLen;
}

bool isValidEmail(const std::string& s) {
    static const std::regex reEmail("^[A-Za-z0-9._%+-]{1,64}@[A-Za-z0-9.-]{1,253}\\.[A-Za-z]{2,63}$");
    if (!withinLength(s, 3, 254)) return false;
    return std::regex_match(s, reEmail);
}

bool isValidUsername(const std::string& s) {
    static const std::regex reUser("^[A-Za-z][A-Za-z0-9_]{2,19}$");
    if (!withinLength(s, 3, 20)) return false;
    return std::regex_match(s, reUser);
}

bool isValidPhone(const std::string& s) {
    static const std::regex reAllowed("^[+0-9() .-]{7,20}$");
    if (!withinLength(s, 7, 20)) return false;
    if (!std::regex_match(s, reAllowed)) return false;
    size_t digits = 0;
    for (char c : s) {
        if (c >= '0' && c <= '9') ++digits;
    }
    return digits >= 7 && digits <= 15;
}

bool isValidISODate(const std::string& s) {
    static const std::regex reDate("^\\d{4}-(0[1-9]|1[0-2])-(0[1-9]|[12]\\d|3[01])$");
    if (!withinLength(s, 10, 10)) return false;
    return std::regex_match(s, reDate);
}

bool isValidPassword(const std::string& s) {
    static const std::regex reAllowed("^[\\S]{8,64}$");
    static const std::regex reLower(".*[a-z].*");
    static const std::regex reUpper(".*[A-Z].*");
    static const std::regex reDigit(".*\\d.*");
    static const std::regex reSpecial(".*[^A-Za-z0-9].*");
    if (!withinLength(s, 8, 64)) return false;
    if (!std::regex_match(s, reAllowed)) return false;
    if (!std::regex_match(s, reLower)) return false;
    if (!std::regex_match(s, reUpper)) return false;
    if (!std::regex_match(s, reDigit)) return false;
    if (!std::regex_match(s, reSpecial)) return false;
    return true;
}

int main() {
    std::vector<std::string> emails = {
        "alice@example.com",
        "alice@@example.com",
        "user.name+tag@sub.domain.co",
        "no-at-symbol.domain",
        "this-is-long_but.valid-123@domain-info.org"
    };
    std::vector<std::string> usernames = {
        "Alice_01",
        "1bad",
        "ab",
        "Valid_User_20",
        "UserNameWithMaxLen20"
    };
    std::vector<std::string> phones = {
        "+1-202-555-0123",
        "12345",
        "(020) 7946 0958",
        "+44 20 7946 0958 12345",
        "202.555.0180"
    };
    std::vector<std::string> dates = {
        "2025-10-14",
        "2025-13-01",
        "2025-02-00",
        "1999-12-31",
        "abcd-ef-gh"
    };
    std::vector<std::string> passwords = {
        "Str0ng!Pwd",
        "weak",
        "NoSpecial123",
        "alllower!!1",
        "GoodPass#2025"
    };

    for (size_t i = 0; i < 5; ++i) {
        std::cout << "Test case #" << (i + 1) << "\n";
        std::cout << "  Email: " << emails[i] << " -> " << (isValidEmail(emails[i]) ? "true" : "false") << "\n";
        std::cout << "  Username: " << usernames[i] << " -> " << (isValidUsername(usernames[i]) ? "true" : "false") << "\n";
        std::cout << "  Phone: " << phones[i] << " -> " << (isValidPhone(phones[i]) ? "true" : "false") << "\n";
        std::cout << "  Date: " << dates[i] << " -> " << (isValidISODate(dates[i]) ? "true" : "false") << "\n";
        // Do not print password values
        std::cout << "  Password valid? -> " << (isValidPassword(passwords[i]) ? "true" : "false") << "\n";
    }
    return 0;
}