#include <iostream>
#include <string>
#include <vector>
#include <regex>

/**
 * @brief Determines the redirect URL based on a regex match.
 * The regex matches a string that starts with one or more alphanumeric characters or hyphens,
 * followed by a period.
 * 
 * @param target The target query parameter string.
 * @return A string indicating redirection to the target if it matches, or to the root ("/") otherwise.
 */
std::string getRedirectUrl(const std::string& target) {
    // Regex: starts with one or more alphanumeric/hyphen chars, followed by a period.
    // The \\. is needed to escape the backslash for the literal dot in the C++ string.
    std::regex pattern("^[a-zA-Z0-9-]+\\..*");

    if (std::regex_match(target, pattern)) {
        return "Redirecting to: " + target;
    } else {
        return "Redirecting to: /";
    }
}

int main() {
    std::vector<std::string> testCases = {
        "example.com",
        "sub-domain.example.org/path?query=1",
        "123-valid.net",
        ".invalid-start.com",
        "/local/path/only"
    };

    std::cout << "CPP Test Cases:" << std::endl;
    for (const auto& testCase : testCases) {
        std::cout << "Input: \"" << testCase << "\" -> Output: " << getRedirectUrl(testCase) << std::endl;
    }

    return 0;
}