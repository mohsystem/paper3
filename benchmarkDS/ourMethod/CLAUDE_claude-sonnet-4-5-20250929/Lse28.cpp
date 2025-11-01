
#include <iostream>
#include <string>
#include <regex>

// Compile regex once at program initialization
// Pattern validates HTTPS URLs with safe character set
static const std::regex URL_PATTERN("^https://[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}/[a-zA-Z0-9/_-]*$");

// Default URL is a constant, not constructed from user input
static const std::string DEFAULT_URL = "https://example.com/default";

/**
 * Validates and returns a redirect URL.
 * The regex validates format only; matched URL is used directly without reconstruction.
 * 
 * @param targetUrl The URL to validate
 * @return The validated target URL or default URL
 */
std::string getRedirectUrl(const std::string& targetUrl) {
    // Input validation: check for excessive length
    if (targetUrl.empty() || targetUrl.length() > 2048) {
        return DEFAULT_URL;
    }
    
    // Validate against whitelist pattern
    // The regex is used only for validation, not construction
    std::smatch match;
    
    // If pattern matches, return the original validated input
    // Not constructing URL from regex groups - using validated input directly
    if (std::regex_match(targetUrl, match, URL_PATTERN)) {
        return targetUrl;
    }
    
    // If validation fails, return safe default
    return DEFAULT_URL;
}

int main() {
    // Test case 1: Valid HTTPS URL
    std::string test1 = "https://trusted-site.com/page";
    std::cout << "Test 1: " << getRedirectUrl(test1) << std::endl;
    
    // Test case 2: Valid HTTPS URL with path
    std::string test2 = "https://secure.example.org/user/profile";
    std::cout << "Test 2: " << getRedirectUrl(test2) << std::endl;
    
    // Test case 3: Invalid - HTTP instead of HTTPS
    std::string test3 = "http://untrusted.com/page";
    std::cout << "Test 3: " << getRedirectUrl(test3) << std::endl;
    
    // Test case 4: Invalid - contains special characters
    std::string test4 = "https://evil.com/redirect?url=javascript:alert(1)";
    std::cout << "Test 4: " << getRedirectUrl(test4) << std::endl;
    
    // Test case 5: Empty input
    std::string test5 = "";
    std::cout << "Test 5: " << getRedirectUrl(test5) << std::endl;
    
    return 0;
}
