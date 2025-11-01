#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Checks the domain of a URL and returns a redirect or 404 message.
 *
 * This function uses simple string manipulation to find the domain. It may not
 * handle all edge cases of complex URLs (e.g., with user info).
 *
 * @param url The URL string to check.
 * @return A string indicating redirection or a 404 error.
 */
std::string redirect_url(const std::string& url) {
    if (url.empty()) {
        return "404 Not Found";
    }

    // Find the protocol separator "://"
    size_t scheme_pos = url.find("://");
    if (scheme_pos == std::string::npos) {
        return "404 Not Found"; // Not a valid absolute URL for our parser
    }

    // The domain starts right after "://"
    size_t domain_start = scheme_pos + 3;

    // The domain ends at the next '/', ':', or '?'
    size_t domain_end = url.find_first_of("/:?", domain_start);

    std::string domain;
    if (domain_end == std::string::npos) {
        // The domain is the rest of the string
        domain = url.substr(domain_start);
    } else {
        domain = url.substr(domain_start, domain_end - domain_start);
    }

    if (domain == "example.com") {
        return "Redirecting to " + url;
    } else {
        return "404 Not Found";
    }
}

int main() {
    std::vector<std::string> test_cases = {
        "https://example.com/some/path",  // Case 1: Match
        "http://google.com/search",       // Case 2: No Match
        "invalid-url-format",             // Case 3: Invalid format
        "https://another-site.net",       // Case 4: Another No Match
        "http://example.com"              // Case 5: Match without path
    };

    std::cout << "--- Running 5 Test Cases ---" << std::endl;
    for (const auto& url : test_cases) {
        std::cout << "Input: " << url << std::endl;
        std::cout << "Output: " << redirect_url(url) << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}