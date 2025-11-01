#include <iostream>
#include <string>
#include <vector>
#include <regex>

/**
 * @brief Determines the redirect URL based on a target parameter.
 * 
 * The regular expression matches a string that starts with one or more 
 * alphanumeric characters or hyphens, followed by a period.
 * This is a security measure to prevent open redirect vulnerabilities.
 * 
 * @param target The target query parameter.
 * @return The target if it matches the safe pattern, otherwise "/" (the root of the site).
 */
std::string getRedirectUrl(const std::string& target) {
    // The regex ensures the target starts like a domain name (e.g., "example.com")
    // and is not a scheme-relative URL (e.g., "//evil.com") or an absolute path (e.g., "/login").
    // Note: C++ uses raw string literals R"(...)" or double backslashes for escaping.
    const std::regex safeRedirectPattern("^[a-zA-Z0-9-]+\\..*");

    // Check for empty input, which is considered invalid.
    if (target.empty()) {
        return "/";
    }

    // std::regex_match checks if the entire string matches the pattern.
    // If the target matches the safe pattern, it's considered safe for redirection.
    if (std::regex_match(target, safeRedirectPattern)) {
        return target;
    }

    // Otherwise, redirect to the root to prevent a potential open redirect attack.
    return "/";
}

int main() {
    std::vector<std::string> testCases = {
        "example.com/home",          // Valid: Standard domain with path
        "sub-domain.net?query=1",    // Valid: Subdomain with query string
        "/local/path",               // Invalid: Does not start with a domain-like pattern
        "justaword",                 // Invalid: No period
        "@bad.com/path"              // Invalid: Starts with a disallowed character
    };

    std::cout << "Running C++ Test Cases:" << std::endl;
    for (const auto& testCase : testCases) {
        std::string result = getRedirectUrl(testCase);
        std::cout << "Input: \"" << testCase << "\", Redirect To: \"" << result << "\"" << std::endl;
    }

    return 0;
}