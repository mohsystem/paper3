#include <iostream>
#include <string>
#include <regex>
#include <vector>

/**
 * Determines the redirect URL based on a regex pattern.
 * WARNING: The logic described in the prompt is vulnerable to Open Redirect attacks.
 * A user can be redirected to any external domain that matches the pattern (e.g., "evil.com").
 * A more secure implementation should validate the domain against a strict allow-list.
 *
 * @param target The target URL, supposedly from a query parameter.
 * @return The URL to redirect to (either the target or "/").
 */
std::string getRedirectUrl(const std::string& target) {
    if (target.empty()) {
        std::cout << "Target is empty. Redirecting to root." << std::endl;
        return "/";
    }
    // Regex: matches a string that starts with one or more alphanumeric characters or hyphens,
    // followed by a period.
    // Using R"()" for raw string literal to avoid escaping backslashes.
    const std::regex pattern(R"(^[a-zA-Z0-9-]+\..*)");

    if (std::regex_match(target, pattern)) {
        std::cout << "Target '" << target << "' matches the pattern. Redirecting to target." << std::endl;
        return target;
    } else {
        std::cout << "Target '" << target << "' does not match the pattern. Redirecting to root." << std::endl;
        return "/";
    }
}

int main() {
    std::vector<std::string> testCases = {
        "dashboard.myapp.com/user/1", // Valid
        "some-service.net/api/v1",    // Valid
        "example.com",                // Valid
        "/settings/profile",          // Invalid
        "malicious.com"               // Valid (demonstrates open redirect risk)
    };
    
    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& testCase : testCases) {
        std::string redirectUrl = getRedirectUrl(testCase);
        std::cout << "=> Final Redirect URL: " << redirectUrl << std::endl;
        std::cout << "-------------------------" << std::endl;
    }

    return 0;
}