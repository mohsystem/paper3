#include <iostream>
#include <string>
#include <vector>
#include <regex>

// C++ does not have a standard library for a web server.
// This code demonstrates the core validation logic.
// In a real application, this function would be called by a web framework
// (like Crow, cpp-httplib, or Boost.Beast) to determine the redirect URL.

/**
 * @brief Validates a target URL against a secure regex and returns the redirect destination.
 * 
 * @param target The target URL to validate.
 * @return The original target URL if it's valid, otherwise "/".
 */
std::string getRedirectUrl(const std::string& target) {
    // Secure regex to validate the target URL.
    // It ensures the protocol is http/https, the domain is example.com or a subdomain,
    // and prevents user-info pass-through (e.g., http://example.com@evil.com).
    try {
        const std::regex url_regex("^https?://(?!.*@)([a-zA-Z0-9-]+\\.)*example\\.com(/.*)?$");
        if (std::regex_match(target, url_regex)) {
            return target;
        }
    } catch (const std::regex_error& e) {
        // In case of a bad regex pattern, default to safe behavior.
        std::cerr << "Regex error: " << e.what() << std::endl;
        return "/";
    }
    
    return "/";
}

int main() {
    // Main class name as per instructions (conceptual in C++)
    class Lse29 {}; 

    // Test cases
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"https://sub.example.com/path/to/resource", "https://sub.example.com/path/to/resource"},
        {"http://example.com", "http://example.com"},
        {"https://www.google.com", "/"},
        {"https://example.com.malicious.com", "/"},
        {"javascript:alert(1)//http://example.com", "/"},
        {"https://user@evil.com?q=example.com", "/"}, // User-info bypass attempt
        {"https://example.com@evil.com/path", "/"}   // Another user-info bypass
    };

    std::cout << "Running C++ test cases..." << std::endl;
    int passed = 0;
    for (const auto& test : testCases) {
        std::string result = getRedirectUrl(test.first);
        std::cout << "Input:    " << test.first << std::endl;
        std::cout << "Output:   " << result << std::endl;
        std::cout << "Expected: " << test.second << std::endl;
        if (result == test.second) {
            std::cout << "Result:   PASS" << std::endl;
            passed++;
        } else {
            std::cout << "Result:   FAIL" << std::endl;
        }
        std::cout << "------------------------------------" << std::endl;
    }

    std::cout << passed << "/" << testCases.size() << " tests passed." << std::endl;

    return 0;
}