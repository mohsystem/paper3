#include <iostream>
#include <regex>
#include <string>

// Compiled once when the module is loaded (global static)
static const std::regex ALLOW_PATTERN(
    R"(^(?i:https://)([A-Za-z0-9-]+\.)*example\.com(:[0-9]{1,5})?(/.*)?$)",
    std::regex::icase
);

static constexpr size_t MAX_URL_LEN = 2048;
static const std::string SAFE_DEFAULT = "https://www.example.com/";

static bool is_allowed(const std::string& url) {
    if (url.empty() || url.size() > MAX_URL_LEN) return false;
    if (url.find('\r') != std::string::npos || url.find('\n') != std::string::npos) return false;
    return std::regex_match(url, ALLOW_PATTERN);
}

// Returns targetUrl if allowed; otherwise safe default. Regex not used to construct output.
std::string resolveRedirect(const std::string& targetUrl, const std::string& defaultUrl) {
    const std::string safeFallback = is_allowed(defaultUrl) ? defaultUrl : SAFE_DEFAULT;
    if (is_allowed(targetUrl)) {
        return targetUrl;
    }
    return safeFallback;
}

int main() {
    // 5 test cases
    const std::pair<std::string, std::string> tests[] = {
        {"https://example.com/path", "https://www.example.com/home"},
        {"https://sub.example.com/", "https://www.example.com/fallback"},
        {"https://evil.com/", "https://www.example.com/fallback"},
        {"http://example.com/", "https://www.example.com/fallback"},
        {"https://example.com/" + std::string(2100, 'a'), "http://insecure.example.com/"},
    };

    int i = 1;
    for (const auto& td : tests) {
        std::string shown = td.first.size() <= 80 ? td.first : (td.first.substr(0, 80) + "...");
        std::cout << "Test " << i++ << ":\n";
        std::cout << "  target = " << shown << "\n";
        std::cout << "  default= " << td.second << "\n";
        std::cout << "  result = " << resolveRedirect(td.first, td.second) << "\n";
    }
    return 0;
}