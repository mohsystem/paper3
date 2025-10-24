#include <iostream>
#include <string>
#include <vector>

static bool isAlpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
static bool isDigit(char c) {
    return (c >= '0' && c <= '9');
}
static bool isLocalAllowed(char c) {
    return isAlpha(c) || isDigit(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-';
}
static bool isDomainLabelChar(char c) {
    return isAlpha(c) || isDigit(c) || c == '-';
}

bool isValidEmail(const std::string& email) {
    if (email.empty() || email.size() > 254) return false;

    size_t at = email.find('@');
    if (at == std::string::npos || at == 0) return false;
    if (email.find('@', at + 1) != std::string::npos) return false;

    std::string local = email.substr(0, at);
    std::string domain = email.substr(at + 1);
    if (local.empty() || domain.empty()) return false;
    if (local.size() > 64) return false;

    if (local.front() == '.' || local.back() == '.') return false;
    if (local.find("..") != std::string::npos) return false;
    for (char c : local) {
        if (!isLocalAllowed(c)) return false;
    }

    if (domain.front() == '.' || domain.back() == '.') return false;
    if (domain.find("..") != std::string::npos) return false;
    for (char c : domain) {
        if (!(isAlpha(c) || isDigit(c) || c == '-' || c == '.')) return false;
    }

    if (domain.find('.') == std::string::npos) return false;

    // Split domain into labels
    std::vector<std::string> labels;
    size_t start = 0;
    while (true) {
        size_t pos = domain.find('.', start);
        if (pos == std::string::npos) {
            labels.push_back(domain.substr(start));
            break;
        }
        labels.push_back(domain.substr(start, pos - start));
        start = pos + 1;
    }
    if (labels.size() < 2) return false;

    for (const auto& label : labels) {
        if (label.empty() || label.size() > 63) return false;
        if (label.front() == '-' || label.back() == '-') return false;
        for (char c : label) {
            if (!isDomainLabelChar(c)) return false;
        }
    }

    const std::string& tld = labels.back();
    if (tld.size() < 2) return false;
    for (char c : tld) {
        if (!isAlpha(c)) return false;
    }

    return true;
}

int main() {
    std::vector<std::string> tests = {
        "user.name+tag@example.com",
        "user@sub.example.co",
        "user..name@example.com",
        "user@-example.com",
        "user@example"
    };

    for (const auto& e : tests) {
        std::cout << e << " -> " << (isValidEmail(e) ? "true" : "false") << "\n";
    }
    return 0;
}