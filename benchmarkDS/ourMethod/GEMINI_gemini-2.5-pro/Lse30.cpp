#include <iostream>
#include <string>
#include <vector>
#include <regex>

// NOTE: This logic is vulnerable to CWE-601: Open Redirect.
// A user can be redirected to any external site that matches this pattern (e.g., "evil-site.com").
// A safer implementation would validate the target against a whitelist of allowed domains or paths.
const std::regex TARGET_PATTERN(R"(^[a-zA-Z0-9-]+\..*)");

/**
 * @brief Determines the redirect URL based on a target parameter.
 *
 * @param target The target parameter from the request.
 * @return The target URL if it's valid, otherwise the root path "/".
 */
std::string getRedirectUrl(const std::string& target) {
    // Rule #1: Ensure all input is validated.
    // An empty target is considered invalid.
    if (target.empty()) {
        return "/";
    }

    if (std::regex_match(target, TARGET_PATTERN)) {
        return target;
    } else {
        return "/";
    }
}

int main() {
    std::vector<std::string> testCases = {
        "example.com",                  // Valid
        "sub-domain.net/path?query=1",  // Valid
        "nodot",                        // Invalid: no dot
        ".startwithdot.com",            // Invalid: starts with dot
        ""                              // Invalid: empty string
    };

    std::cout << "Running CPP test cases..." << std::endl;
    int i = 1;
    for (const auto& target : testCases) {
        std::string redirectUrl = getRedirectUrl(target);
        std::cout << "Test Case " << i++ << ": target=\"" << target 
                  << "\", redirect_to=\"" << redirectUrl << "\"" << std::endl;
    }
    
    return 0;
}