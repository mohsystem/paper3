#include <iostream>
#include <unordered_set>
#include <string>
#include <algorithm>
#include <cctype>

static std::string toLowerTrim(const std::string& s) {
    size_t start = 0, end = s.size();
    while (start < end && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    std::string out = s.substr(start, end - start);
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return out;
}

static bool isAllowedLocal(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
}
static bool isAllowedDomain(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '.' || c == '-';
}

static bool isValidEmail(const std::string& email) {
    if (email.empty() || email.size() > 254) return false;
    auto at_pos = email.find('@');
    if (at_pos == std::string::npos) return false;
    if (email.find('@', at_pos + 1) != std::string::npos) return false;
    std::string local = email.substr(0, at_pos);
    std::string domain = email.substr(at_pos + 1);
    if (local.empty() || local.size() > 64 || domain.empty()) return false;
    for (char c : local) if (!isAllowedLocal(c)) return false;
    for (char c : domain) if (!isAllowedDomain(c)) return false;
    if (domain.find('.') == std::string::npos) return false;
    // TLD at least 2 letters
    auto last_dot = domain.find_last_of('.');
    if (last_dot == std::string::npos || last_dot + 2 >= domain.size()) return false;
    for (size_t i = last_dot + 1; i < domain.size(); ++i) {
        if (!std::isalpha(static_cast<unsigned char>(domain[i]))) return false;
    }
    return true;
}

// Unsubscribe function: accepts input as parameters and returns output
static std::string unsubscribe(const std::string& email,
                               std::unordered_set<std::string>& database,
                               std::unordered_set<std::string>& mailingList) {
    std::string normalized = toLowerTrim(email);
    if (!isValidEmail(normalized)) {
        return "Invalid email address.";
    }
    auto it = database.find(normalized);
    if (it != database.end()) {
        database.erase(it);
        mailingList.erase(normalized);
        return "Successfully unsubscribed.";
    } else {
        return "Email not subscribed.";
    }
}

int main() {
    std::unordered_set<std::string> database = {
        "alice@example.com",
        "bob@example.org",
        "upper@example.com",
        "carol+news@sub.example.co",
        "dave@example.net"
    };
    std::unordered_set<std::string> mailingList = {
        "alice@example.com",
        "bob@example.org",
        "upper@example.com",
        "carol+news@sub.example.co"
    };

    // 5 test cases
    std::cout << unsubscribe("Alice@Example.com", database, mailingList) << "\n";
    std::cout << unsubscribe("unknown@example.com", database, mailingList) << "\n";
    std::cout << unsubscribe("invalid-email", database, mailingList) << "\n";
    std::cout << unsubscribe("BOB@EXAMPLE.ORG", database, mailingList) << "\n";
    std::cout << unsubscribe("dave@example.net", database, mailingList) << "\n";

    return 0;
}