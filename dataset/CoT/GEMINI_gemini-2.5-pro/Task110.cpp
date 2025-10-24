#include <iostream>
#include <string>
#include <regex>
#include <vector>

/**
 * Validates a URL using a regular expression.
 * This regex checks for http, https, or ftp schemes, a valid domain name,
 * and an optional path.
 *
 * @param url The string to validate.
 * @return true if the URL is valid, false otherwise.
 */
bool isValidURL(const std::string& url) {
    // Handle empty input string
    if (url.empty()) {
        return false;
    }

    // Regex for a standard URL (http, https, ftp)
    // C++ requires backslashes to be escaped in string literals.
    // ^(https?|ftp)://    - Scheme (http, https, ftp)
    // [a-zA-Z0-9.-]+      - Domain name characters
    // \.[a-zA-Z]{2,}      - Top-level domain (at least 2 letters)
    // (?:/[^\s]*)?$       - Optional path (any non-whitespace characters)
    const std::regex url_regex(
        "^(https?|ftp)://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(?:/[^\\s]*)?$",
        std::regex_constants::ECMAScript
    );

    return std::regex_match(url, url_regex);
}

int main() {
    std::vector<std::string> testUrls = {
        "https://www.example.com",
        "http://example.co.uk/path/to/page?query=123",
        "ftp://ftp.example.com/resource",
        "htp://invalid-scheme.com", // Invalid scheme
        "https://.invalid-domain.com", // Invalid domain
        "https://domain-without-tld", // Invalid TLD
        "Just a string", // Not a URL
        "" // Empty string
    };
    
    std::cout << "Running URL validation test cases in C++:" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "URL: \"" << url << "\" -> " << (isValidURL(url) ? "Valid" : "Invalid") << std::endl;
    }

    return 0;
}