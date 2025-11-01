#include <iostream>
#include <string>
#include <regex>
#include <vector>

// The regular expression is compiled once as a global constant.
const std::regex URL_PATTERN("^https?://(www\\.)?example\\.com/.*$");
const std::string DEFAULT_URL = "https://default-redirect.com";

/**
 * Determines the redirect URL based on a pattern match.
 *
 * @param targetUrl The URL to check.
 * @return The targetUrl if it matches the pattern, otherwise the default URL.
 */
std::string getRedirectUrl(const std::string& targetUrl) {
    // If the target URL matches, it is used as the redirect target.
    if (std::regex_match(targetUrl, URL_PATTERN)) {
        return targetUrl;
    }
    // If the target URL does not match, a default URL is used.
    return DEFAULT_URL;
}

int main() {
    std::vector<std::string> testUrls = {
        "https://www.example.com/path/to/resource", // Should match
        "http://example.com/another-page",          // Should match
        "https://www.google.com",                   // Should not match
        "https://subdomain.example.com/page",       // Should not match
        "ftp://example.com/file"                    // Should not match
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (const auto& url : testUrls) {
        std::string redirectUrl = getRedirectUrl(url);
        std::cout << "Input: " << url << " -> Redirect: " << redirectUrl << std::endl;
    }

    return 0;
}