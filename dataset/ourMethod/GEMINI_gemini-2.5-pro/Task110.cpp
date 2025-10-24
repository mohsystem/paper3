#include <iostream>
#include <string>
#include <regex>
#include <vector>

/**
 * @brief Validates if a given string is a well-formed URL.
 *
 * @param urlString The string to validate.
 * @return true if the string is a valid URL, false otherwise.
 */
bool isValidURL(const std::string& urlString) {
    if (urlString.empty()) {
        return false;
    }

    // A comprehensive regex for URL validation.
    // It checks for a scheme, domain/IP, optional port, and optional path.
    // Raw string literal R"()" is used to avoid excessive backslash escaping.
    const std::regex url_regex(
        R"(^(?:http|ftp)s?://(?:(?:[A-Z0-9](?:[A-Z0-9-]{0,61}[A-Z0-9])?\.)+(?:[A-Z]{2,6}\.?|[A-Z0-9-]{2,}\.?)|localhost|\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3})(?::\d+)?(?:/?|[/?]\S*)$)",
        std::regex_constants::icase
    );

    return std::regex_match(urlString, url_regex);
}

int main() {
    std::vector<std::string> testUrls = {
        "https://www.google.com",
        "http://example.com/path?query=1",
        "ftp://ftp.is.co.za/rfc/rfc1808.txt",
        "www.invalid.com",
        "https://",
        "http://exa mple.com"
    };

    std::cout << "Running URL validation tests:" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "URL: \"" << url << "\" -> Valid: " << std::boolalpha << isValidURL(url) << std::endl;
    }

    return 0;
}