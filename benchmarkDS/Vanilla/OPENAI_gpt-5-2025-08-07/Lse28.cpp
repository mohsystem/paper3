#include <regex>
#include <string>
#include <iostream>
#include <vector>

// Compile the regular expression once when the module is loaded
static const std::regex ALLOWED_URL("^https?://(?:www\\.)?example\\.com(?:/.*)?$", std::regex::icase);

std::string redirect(const std::string& targetUrl, const std::string& defaultUrl) {
    if (std::regex_match(targetUrl, ALLOWED_URL)) {
        return targetUrl;
    }
    return defaultUrl;
}

int main() {
    std::string defaultUrl = "https://example.com/default";
    std::vector<std::string> tests = {
        "https://example.com/path?a=1",
        "http://www.example.com",
        "https://evil.com/phish",
        "ftp://example.com/file",
        "https://sub.example.com/page"
    };

    for (const auto& t : tests) {
        std::cout << t << " -> " << redirect(t, defaultUrl) << std::endl;
    }
    return 0;
}