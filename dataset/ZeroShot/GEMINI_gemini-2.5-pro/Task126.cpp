#include <iostream>
#include <string>
#include <map>

// A secure, allowlisted map of page keys to their full URLs.
const std::map<std::string, std::string> ALLOWED_REDIRECTS = {
    {"home", "https://example.com/home"},
    {"about", "https://example.com/about"},
    {"contact", "https://example.com/contact-us"},
    {"products", "https://example.com/products"}
};
const std::string DEFAULT_URL = "https://example.com/error";

/**
 * Securely gets a redirect URL based on a predefined allowlist.
 * Prevents open redirect vulnerabilities by looking up the key in a safe map.
 *
 * @param pageKey The user-provided key for the desired page.
 * @return The corresponding URL from the allowlist, or a default URL if the key is not found.
 */
std::string getRedirectUrl(const std::string& pageKey) {
    auto it = ALLOWED_REDIRECTS.find(pageKey);
    if (it != ALLOWED_REDIRECTS.end()) {
        // Key was found, return the corresponding URL
        return it->second;
    }
    // Key was not found, return the default URL
    return DEFAULT_URL;
}

int main() {
    std::string testCases[] = {
        "home",              // Test case 1: Valid input
        "about",             // Test case 2: Another valid input
        "admin",             // Test case 3: Invalid input
        "www.evil.com",      // Test case 4: Malicious input attempt
        ""                   // Test case 5: Empty input
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (const auto& testCase : testCases) {
        std::string redirectUrl = getRedirectUrl(testCase);
        std::cout << "Input: '" << testCase << "' -> Redirecting to: " << redirectUrl << std::endl;
    }

    return 0;
}