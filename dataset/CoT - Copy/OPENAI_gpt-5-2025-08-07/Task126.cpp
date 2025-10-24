#include <iostream>
#include <unordered_map>
#include <string>
#include <regex>
#include <algorithm>
#include <cctype>

// Step 1: Purpose: return a redirect path based on user input
// Step 2: Security: whitelist routing and input validation
// Step 3: Implement sanitize and strict map lookup
// Step 4: Review: no untrusted URL usage
// Step 5: Output: only safe predefined routes or /error

static std::unordered_map<std::string, std::string> ROUTES = {
    {"home", "/home"},
    {"profile", "/user/profile"},
    {"settings", "/settings"},
    {"help", "/help"},
    {"logout", "/logout"}
};

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    if (start == s.size()) return "";
    size_t end = s.size() - 1;
    while (end > start && std::isspace(static_cast<unsigned char>(s[end]))) --end;
    return s.substr(start, end - start + 1);
}

static std::string sanitize(const std::string& input) {
    std::string s = trim(input);
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    static const std::regex allowed("^[a-z0-9_-]+$");
    if (!std::regex_match(s, allowed)) return "";
    return s;
}

std::string getRedirectTarget(const std::string& userInput) {
    std::string key = sanitize(userInput);
    auto it = ROUTES.find(key);
    if (it != ROUTES.end()) return it->second;
    return "/error";
}

int main() {
    const std::string tests[5] = {"home", "HoMe", " profile ", "../admin", "unknown"};
    for (const auto& t : tests) {
        std::cout << "Input: \"" << t << "\" -> Redirect: " << getRedirectTarget(t) << "\n";
    }
    return 0;
}