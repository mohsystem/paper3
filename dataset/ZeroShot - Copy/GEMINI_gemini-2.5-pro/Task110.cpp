#include <iostream>
#include <string>
#include <regex>
#include <vector>

/**
 * Validates a URL using C++ standard regex library.
 * The regex is crafted to be reasonably strict and secure, checking for a valid
 * scheme, a domain name or IP address conforming to common rules (e.g., no
 * leading/trailing hyphens in domain labels), and optional port/path/query/fragment.
 * It also enforces a maximum length to prevent potential ReDoS attacks with very long strings.
 *
 * @param urlString The URL string to validate.
 * @return true if the URL is valid, false otherwise.
 */
bool validateUrl(const std::string& urlString) {
    // 1. Check for unreasonable length.
    if (urlString.length() > 2048) {
        return false;
    }

    // 2. A practical regex for URL validation based on RFCs.
    // This is not exhaustive for all edge cases but covers the vast majority
    // of valid URLs securely.
    const std::regex url_regex(
        R"(^(https?|ftp)://)"                                       // Scheme
        R"((([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]{2,63}|)" // Domain name
        R"(localhost|)"                                             // or localhost
        R"(((\d{1,3}\.){3}\d{1,3})))"                               // or IP v4
        R"((:\d{1,5})?)"                                            // Optional Port
        R"((/[^?#]*)?)"                                             // Optional Path
        R"((\?[^#]*)?)"                                             // Optional Query
        R"((#.*)?$)"                                                // Optional Fragment
        , std::regex_constants::icase); // Case-insensitive match

    // 3. Perform the match.
    return std::regex_match(urlString, url_regex);
}

int main() {
    std::vector<std::string> testUrls = {
        "https://www.google.com",
        "http://example.com/path?name=value#fragment",
        "ftp://127.0.0.1:21/resource",
        "www.google.com", // Invalid: no scheme
        "http://invalid-.com", // Invalid: domain label ends with hyphen
        "garbage" // Invalid: not a URL
    };

    std::cout << "CPP URL Validation:" << std::endl;
    for (const auto& url : testUrls) {
        std::cout << "URL: \"" << url << "\" -> " << (validateUrl(url) ? "Valid" : "Invalid") << std::endl;
    }

    return 0;
}