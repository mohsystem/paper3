#include <iostream>
#include <string>
#include <regex>
#include <vector>

// The default URL to use if the target URL does not match the whitelist.
const std::string DEFAULT_URL = "https://example.com/default";

/**
 * Determines the redirect URL based on a whitelist pattern.
 * If the target URL matches the pre-compiled regular expression, it is returned.
 * Otherwise, a default URL is returned.
 *
 * @param targetUrl The URL to validate for redirection.
 * @return The targetUrl if it's valid, otherwise the default URL.
 */
std::string getRedirectUrl(const std::string& targetUrl) {
    // The regular expression is compiled once on the first function call
    // due to the 'static' keyword. It whitelists URLs from a specific, safe domain.
    static const std::regex safeUrlPattern("^https://safe\\.example\\.com/([a-zA-Z0-9/\\-]+)$");

    // The regular expression is used to match the target URL.
    // std::regex_match ensures the entire string matches the pattern.
    if (std::regex_match(targetUrl, safeUrlPattern)) {
        // If the target URL matches, it is used as the redirect target.
        // The regex is not used to construct the URL, preventing manipulation.
        return targetUrl;
    } else {
        // If the target URL does not match, a default URL is used.
        return DEFAULT_URL;
    }
}

// In C++, the main class Lse28 is not a standard convention like in Java.
// The code is organized into functions.
int main() {
    std::vector<std::string> testUrls = {
        // Test Case 1: Valid URL
        "https://safe.example.com/valid-path",
        // Test Case 2: Valid URL with sub-path
        "https://safe.example.com/user/profile-123",
        // Test Case 3: Invalid domain
        "https://unsafe.example.com/path",
        // Test Case 4: Invalid characters (query parameters are not allowed by the regex)
        "https://safe.example.com/path?query=malicious",
        // Test Case 5: URL trying to bypass with different protocol or domain format
        "http://safe.example.com/path"
    };
    
    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "Input:  " << url << std::endl;
        std::cout << "Output: " << getRedirectUrl(url) << std::endl;
        std::cout << "--------------------" << std::endl;
    }
    
    return 0;
}