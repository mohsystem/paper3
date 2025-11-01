#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Checks if a URL is from example.com and simulates a server response.
 *
 * This function implements the core validation logic. In a real web application,
 * it would be part of a request handler that generates an HTTP response.
 * Here, it prints the simulated response to the console.
 *
 * @param url The URL to validate.
 */
void redirect_invalid_url(const std::string& url) {
    // Rule #1: Input validation based on a strict whitelist of URL prefixes.
    // This approach is safer than attempting to parse complex URLs manually
    // without a robust, vetted library. It prevents common bypasses.
    const std::string valid_http_prefix = "http://example.com/";
    const std::string valid_https_prefix = "https://example.com/";
    const std::string valid_http_exact = "http://example.com";
    const std::string valid_https_exact = "https://example.com";

    bool is_valid = false;
    // Using string::compare for a prefix check is both safe and efficient.
    if (url == valid_http_exact ||
        url == valid_https_exact ||
        url.compare(0, valid_http_prefix.length(), valid_http_prefix) == 0 ||
        url.compare(0, valid_https_prefix.length(), valid_https_prefix) == 0) {
        is_valid = true;
    }

    if (is_valid) {
        std::cout << "Status: 302 Found" << std::endl;
        std::cout << "Location: " << url << std::endl << std::endl;
        // This simulates a redirect response.
    } else {
        std::cout << "Status: 400 Bad Request" << std::endl;
        std::cout << "Error: Redirects are only allowed to example.com." << std::endl << std::endl;
        // This simulates an error response.
    }
}

int main() {
    std::cout << "C++ does not have a standard built-in web server." << std::endl;
    std::cout << "This program demonstrates the required URL validation logic for the redirect." << std::endl;
    std::cout << "It simulates the server's response to different URLs." << std::endl << std::endl;

    // 5 Test Cases
    const std::vector<std::string> test_urls = {
        // Test case 1: Valid HTTP URL
        "http://example.com",
        // Test case 2: Valid HTTPS URL with a path
        "https://example.com/some/path",
        // Test case 3: Invalid domain
        "http://google.com",
        // Test case 4: Invalid domain that looks similar
        "http://example.com.malicious.com",
        // Test case 5: Invalid scheme
        "javascript:alert('XSS')"
    };

    for (const auto& url : test_urls) {
        std::cout << "--- Testing URL: \"" << url << "\" ---" << std::endl;
        redirect_invalid_url(url);
    }
    
    return 0;
}