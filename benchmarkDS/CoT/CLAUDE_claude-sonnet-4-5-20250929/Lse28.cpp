
#include <iostream>
#include <string>
#include <regex>

class Lse28 {
private:
    // Compile regex once at class initialization
    // Using a simple, bounded pattern to prevent ReDoS
    static const std::regex URL_PATTERN;
    static const std::string DEFAULT_URL;
    
public:
    /**
     * Validates and returns redirect URL based on pattern matching
     * @param targetUrl The URL to validate
     * @return Valid redirect URL or default URL
     */
    static std::string getRedirectUrl(const std::string& targetUrl) {
        // Empty check
        if (targetUrl.empty()) {
            return DEFAULT_URL;
        }
        
        // Trim whitespace
        std::string trimmedUrl = targetUrl;
        trimmedUrl.erase(0, trimmedUrl.find_first_not_of(" \\t\\n\\r\\f\\v"));
        trimmedUrl.erase(trimmedUrl.find_last_not_of(" \\t\\n\\r\\f\\v") + 1);
        
        // Validate length to prevent DoS
        if (trimmedUrl.length() > 2048) {
            return DEFAULT_URL;
        }
        
        // Match against compiled pattern
        if (std::regex_match(trimmedUrl, URL_PATTERN)) {
            return trimmedUrl;
        }
        
        // Default fallback
        return DEFAULT_URL;
    }
};

// Static member initialization
const std::regex Lse28::URL_PATTERN(
    R"(^https://([a-zA-Z0-9-]+\\.)*example\\.com(/[a-zA-Z0-9-._~:/?#\\[\\]@!$&'()*+,;=]*)?$)"
);
const std::string Lse28::DEFAULT_URL = "https://www.example.com/default";

// Test cases
int main() {
    std::cout << "Test 1: " << Lse28::getRedirectUrl("https://www.example.com/page") << std::endl;
    std::cout << "Test 2: " << Lse28::getRedirectUrl("https://sub.example.com/resource") << std::endl;
    std::cout << "Test 3: " << Lse28::getRedirectUrl("https://malicious.com/phishing") << std::endl;
    std::cout << "Test 4: " << Lse28::getRedirectUrl("") << std::endl;
    std::cout << "Test 5: " << Lse28::getRedirectUrl("https://example.com/path/to/page?query=value") << std::endl;
    
    return 0;
}
