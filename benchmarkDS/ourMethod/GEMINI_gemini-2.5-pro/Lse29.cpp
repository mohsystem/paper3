#include <iostream>
#include <string>
#include <regex>

// This code requires the cpp-httplib single-header library.
// Download "httplib.h" from https://github.com/yhirose/cpp-httplib
// and place it in the same directory or in your include path.
#include "httplib.h"

// A regex that matches URLs where the host contains "example.com".
const std::regex ALLOWED_DOMAIN_REGEX(R"(^https?://[^/]*example\.com.*)");

/**
 * @brief Determines the redirect URL based on a target parameter.
 *
 * @param target The target URL to validate.
 * @return The validated target URL if it matches the regex, otherwise "/".
 */
std::string getRedirectUrl(const std::string& target) {
    if (!target.empty()) {
        if (std::regex_match(target, ALLOWED_DOMAIN_REGEX)) {
            return target;
        }
    }
    return "/";
}

void runTests() {
    std::cout << "\n--- Running 5 Test Cases ---" << std::endl;
    
    // Test Case 1: Valid URL
    std::string test1 = "http://example.com";
    std::cout << "1. (Valid) " << test1 << " -> " << getRedirectUrl(test1) << std::endl;

    // Test Case 2: Valid URL with subdomain and path
    std::string test2 = "http://sub.example.com/some/path";
    std::cout << "2. (Valid) " << test2 << " -> " << getRedirectUrl(test2) << std::endl;
    
    // Test Case 3: Invalid domain
    std::string test3 = "http://another-domain.com";
    std::cout << "3. (Invalid) " << test3 << " -> " << getRedirectUrl(test3) << std::endl;

    // Test Case 4: Invalid URL with tricky parameter
    std::string test4 = "http://evil.com/?q=example.com";
    std::cout << "4. (Invalid) " << test4 << " -> " << getRedirectUrl(test4) << std::endl;
    
    // Test Case 5: Empty target
    std::cout << "5. (Empty target) \"\" -> " << getRedirectUrl("") << std::endl;

    std::cout << "--- Test Cases Finished ---\n" << std::endl;
}

int main() {
    runTests();

    httplib::Server svr;

    svr.Get("/", [](const httplib::Request& req, httplib::Response& res) {
        std::string target;
        if (req.has_param("target")) {
            target = req.get_param_value("target");
        }

        std::string redirect_url = getRedirectUrl(target);

        if (redirect_url == "/") {
            // For invalid or missing targets, show a simple homepage.
            res.set_content("Welcome. Provide a valid 'target' parameter pointing to a URL "
                            "at example.com to be redirected.", "text/plain");
        } else {
            res.set_redirect(redirect_url.c_str(), 302);
        }
    });

    int port = 8081;
    std::cout << "Server starting on port " << port << "..." << std::endl;
    std::cout << "Test with a browser, e.g., http://localhost:8081/?target=http://example.com" << std::endl;
    svr.listen("0.0.0.0", port);

    return 0;
}