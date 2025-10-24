#include <iostream>
#include <string>
#include <map>

/**
 * Returns a redirect URL based on a predefined safe list.
 * This prevents open redirect vulnerabilities by never using the input
 * to construct the URL.
 *
 * @param pageKey A string key representing the desired page.
 * @return A hardcoded, safe URL string.
 */
std::string getRedirectUrl(const std::string& pageKey) {
    static const std::map<std::string, std::string> urlMap = {
        {"home", "https://example.com/home"},
        {"profile", "https://example.com/user/profile"},
        {"settings", "https://example.com/user/settings"},
        {"login", "https://example.com/auth/login"}
    };
    static const std::string defaultUrl = "https://example.com/404";

    if (pageKey.empty()) {
        return defaultUrl;
    }

    auto it = urlMap.find(pageKey);
    if (it != urlMap.end()) {
        return it->second;
    }

    return defaultUrl;
}

int main() {
    // Test cases
    const char* testInputs[] = {
        "home",
        "settings",
        "dashboard", // Invalid key
        "",          // Empty key
        "//malicious-site.com" // Malicious input attempt
    };

    std::cout << "Running test cases for getRedirectUrl:" << std::endl;
    for (const auto& input : testInputs) {
        std::cout << "Input: \"" << input << "\" -> Redirect URL: " << getRedirectUrl(input) << std::endl;
    }
    
    // Additional test for a different kind of invalid input
    std::string longInvalidInput = "this_is_a_very_long_invalid_input_string";
    std::cout << "Input: \"" << longInvalidInput << "\" -> Redirect URL: " << getRedirectUrl(longInvalidInput) << std::endl;

    return 0;
}