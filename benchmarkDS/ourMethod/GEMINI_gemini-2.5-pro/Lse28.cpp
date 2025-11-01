#include <iostream>
#include <string>
#include <regex>
#include <vector>

// The default URL to use if the target URL does not match the safe pattern.
const std::string DEFAULT_URL = "https://default.example.com";

/**
 * Determines the redirect URL based on a whitelist pattern.
 *
 * @param targetUrl The URL to check.
 * @return The targetUrl if it matches the safe pattern, otherwise the default URL.
 */
std::string getRedirectUrl(const std::string& targetUrl) {
    // The regular expression is a static const, ensuring it is compiled only once
    // on the first call to this function. This is thread-safe since C++11.
    static const std::regex SAFE_URL_PATTERN("^https://safe\\.example\\.com/.*$");

    // Rule #1: Validate input. Check for empty strings.
    if (targetUrl.empty()) {
        return DEFAULT_URL;
    }

    // If the target URL matches the pre-compiled regex, it is considered safe.
    if (std::regex_match(targetUrl, SAFE_URL_PATTERN)) {
        return targetUrl;
    }

    // If the target URL does not match, return the default URL.
    return DEFAULT_URL;
}

int main() {
    std::vector<std::string> testUrls = {
        // Test Case 1: Valid URL that should be allowed.
        "https://safe.example.com/user/profile",
        // Test Case 2: URL from an untrusted domain.
        "https://unsafe.example.com/user/profile",
        // Test Case 3: URL with an insecure protocol.
        "http://safe.example.com/user/profile",
        // Test Case 4: Empty input string.
        ""
        // Test Case 5 for NULL is handled by function signature in C++
        // or by using a different type like std::optional. Here we focus on std::string.
        // A direct call with "" covers the "no value" case for strings.
    };
    
    // Additional test to simulate a "null" or uninitialized input case
    std::string uninitialized_url;

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& url : testUrls) {
        std::string redirectUrl = getRedirectUrl(url);
        std::cout << "Input:  \"" << url << "\"" << std::endl;
        std::cout << "Output: \"" << redirectUrl << "\"" << std::endl << std::endl;
    }
    
    // Test Case 5: Uninitialized/empty string (equivalent of some "null" cases)
    std::cout << "Input:  \"\" (uninitialized)" << std::endl;
    std::cout << "Output: \"" << getRedirectUrl(uninitialized_url) << "\"" << std::endl << std::endl;

    return 0;
}