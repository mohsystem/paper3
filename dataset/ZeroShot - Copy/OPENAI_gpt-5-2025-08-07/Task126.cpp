#include <iostream>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cctype>

static const std::unordered_map<std::string, std::string> ROUTES = {
    {"home", "/home"},
    {"index", "/home"},
    {"main", "/home"},
    {"about", "/about"},
    {"contact", "/contact"},
    {"dashboard", "/dashboard"},
    {"settings", "/settings"},
    {"help", "/help"}
};

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) start++;
    if (start == s.size()) return "";
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) end--;
    return s.substr(start, end - start);
}

std::string redirect_user(const std::string& input) {
    const std::string DEFAULT_HOME = "/home";
    const std::string ERROR_PAGE = "/error";

    if (&input == nullptr) { // Defensive; though references are never null
        return DEFAULT_HOME;
    }

    std::string s = trim(input);
    if (s.empty() || s.size() > 100) {
        return ERROR_PAGE;
    }

    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    if (s.rfind("http:", 0) == 0 || s.rfind("https:", 0) == 0 || s.find("://") != std::string::npos || s.rfind("//", 0) == 0) {
        return ERROR_PAGE;
    }

    for (unsigned char ch : s) {
        if (!((ch >= 'a' && ch <= 'z') || (ch >= '0' && ch <= '9') || ch == '-' || ch == '_')) {
            return ERROR_PAGE;
        }
    }

    auto it = ROUTES.find(s);
    if (it != ROUTES.end()) {
        return it->second;
    }
    return ERROR_PAGE;
}

int main() {
    const std::string tests[5] = {"home", "ABOUT", "Contact", "http://evil.com", "settings"};
    for (const auto& t : tests) {
        std::cout << "Input: " << t << " -> " << redirect_user(t) << "\n";
    }
    return 0;
}