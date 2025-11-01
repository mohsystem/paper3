
#include <iostream>
#include <regex>
#include <string>

// Compile the regular expression once (global variable)
static const std::regex URL_PATTERN("^https?://[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}(/.*)?$");
static const std::string DEFAULT_URL = "https://www.default.com";

std::string getRedirectUrl(const std::string& targetUrl) {
    // If the target URL matches the pattern, use it as redirect target
    if (std::regex_match(targetUrl, URL_PATTERN)) {
        return targetUrl;
    }
    
    // If the target URL does not match, use default URL
    return DEFAULT_URL;
}

int main() {
    // Test case 1: Valid HTTP URL
    std::cout << "Test 1: " << getRedirectUrl("http://www.example.com") << std::endl;
    
    // Test case 2: Valid HTTPS URL with path
    std::cout << "Test 2: " << getRedirectUrl("https://www.example.com/path/to/page") << std::endl;
    
    // Test case 3: Valid URL with subdomain
    std::cout << "Test 3: " << getRedirectUrl("https://subdomain.example.com") << std::endl;
    
    // Test case 4: Invalid URL (missing protocol)
    std::cout << "Test 4: " << getRedirectUrl("www.example.com") << std::endl;
    
    // Test case 5: Invalid URL (random text)
    std::cout << "Test 5: " << getRedirectUrl("not-a-valid-url") << std::endl;
    
    return 0;
}
