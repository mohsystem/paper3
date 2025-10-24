#include <iostream>
#include <string>
#include <regex>
#include <vector>

/**
 * Validates a URL string using a regular expression.
 * Checks for http/https protocol and a valid domain structure.
 * @param url The URL string to validate.
 * @return true if the URL is valid, false otherwise.
 */
bool isValidURL(const std::string& url) {
    // Regex to check for a valid URL format.
    // It requires http or https, a domain name with at least one dot, and a TLD of 2+ letters.
    const std::regex pattern(
        "^(https?://)" // http:// or https://
        "([a-zA-Z0-9-]+\\.)+[a-zA-Z]{2,}" // domain name...
        "(:\\d+)?" // optional port
        "(/.*)?$" // optional path
    );
    return std::regex_match(url, pattern);
}

int main() {
    std::vector<std::string> testUrls = {
        "https://www.google.com",
        "http://example.org/path?name=value",
        "ftp://example.com",
        "https://example..com",
        "http://invalid",
        "just a string"
    };

    std::cout << "--- C++ URL Validation ---" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "URL: \"" << url << "\" is " << (isValidURL(url) ? "valid" : "invalid") << std::endl;
    }

    return 0;
}