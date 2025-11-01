#include <iostream>
#include <string>
#include <regex>
#include <vector>

/**
 * Determines the redirect URL based on a whitelist pattern.
 * The regex is compiled once as a static local variable.
 * If the target URL matches the pattern, it is returned.
 * Otherwise, a default URL is returned.
 *
 * @param targetUrl The URL to validate.
 * @param defaultUrl The fallback URL if validation fails.
 * @return The validated targetUrl or the defaultUrl.
 */
std::string getRedirectUrl(const std::string& targetUrl, const std::string& defaultUrl) {
    // The regular expression is compiled once when the function is first called
    // due to the 'static' keyword.
    // This regex validates that the URL is on the example.com domain.
    static const std::regex ALLOWED_URL_PATTERN("^https?://(www\\.)?example\\.com/.*$");

    if (targetUrl.empty()) {
        return defaultUrl;
    }
    
    // If the target URL matches the allowed pattern, it is used.
    if (std::regex_match(targetUrl, ALLOWED_URL_PATTERN)) {
        return targetUrl;
    } else {
        // Otherwise, the default URL is used.
        return defaultUrl;
    }
}

int main() {
    std::string defaultUrl = "https://example.com/default";

    // Test Cases
    std::vector<std::string> testUrls = {
        "https://example.com/some/path",          // Case 1: Valid HTTPS URL
        "http://www.example.com/another/page.html",// Case 2: Valid HTTP URL with www
        "https://evil.com/some/path",             // Case 3: Invalid domain
        "https://example.com.evil.com/path",      // Case 4: Invalid domain (tricky)
        "not a url"                               // Case 5: Invalid format
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (size_t i = 0; i < testUrls.size(); ++i) {
        const std::string& target = testUrls[i];
        std::string redirect = getRedirectUrl(target, defaultUrl);
        std::cout << "Test Case " << i + 1 << ":" << std::endl;
        std::cout << "  Target:   " << target << std::endl;
        std::cout << "  Redirect: " << redirect << std::endl << std::endl;
    }

    return 0;
}