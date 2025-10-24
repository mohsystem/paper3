
#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <cctype>
#include <algorithm>

// URL validator class with security-focused validation
class URLValidator {
private:
    // Maximum URL length to prevent excessive memory use and DoS
    static constexpr size_t MAX_URL_LENGTH = 2048;
    
    // Validate URL length first to prevent regex DoS attacks
    bool validateLength(const std::string& url) const {
        return !url.empty() && url.length() <= MAX_URL_LENGTH;
    }
    
    // Validate scheme to ensure only safe protocols
    bool validateScheme(const std::string& url) const {
        // Only allow http and https schemes for security
        return url.compare(0, 7, "http://") == 0 || 
               url.compare(0, 8, "https://") == 0;
    }
    
    // Check for null bytes which can cause security issues
    bool containsNullByte(const std::string& url) const {
        return url.find('\\0') != std::string::npos;
    }
    
    // Validate domain name characters
    bool validateDomainChars(const std::string& domain) const {
        for (char c : domain) {
            // Allow alphanumeric, dots, hyphens only
            if (!std::isalnum(static_cast<unsigned char>(c)) && 
                c != '.' && c != '-' && c != ':') {
                return false;
            }
        }
        return true;
    }
    
public:
    // Main validation method with comprehensive security checks
    bool validate(const std::string& url) const {
        // Input validation: check for empty or oversized input
        if (!validateLength(url)) {
            return false;
        }
        
        // Security: prevent null byte injection attacks
        if (containsNullByte(url)) {
            return false;
        }
        
        // Security: only allow safe protocols (http/https)
        if (!validateScheme(url)) {
            return false;
        }
        
        // Extract domain portion for validation
        size_t schemeEnd = url.find("://");
        if (schemeEnd == std::string::npos) {
            return false;
        }
        
        size_t domainStart = schemeEnd + 3;
        size_t domainEnd = url.find('/', domainStart);
        if (domainEnd == std::string::npos) {
            domainEnd = url.length();
        }
        
        // Bounds check to prevent overflow
        if (domainStart >= url.length()) {
            return false;
        }
        
        std::string domain = url.substr(domainStart, domainEnd - domainStart);
        
        // Validate domain is not empty
        if (domain.empty() || domain.length() > 253) {
            return false;
        }
        
        // Security: validate domain characters to prevent injection
        if (!validateDomainChars(domain)) {
            return false;
        }
        
        // Use regex for comprehensive validation with bounded input
        // Pattern validates: scheme, optional auth, domain, optional port, path, query, fragment
        std::regex urlPattern(
            "^https?://"                                    // scheme
            "(?:[a-zA-Z0-9-]+(?:\\\\.[a-zA-Z0-9-]+)*)"       // domain
            "(?::[0-9]{1,5})?"                             // optional port
            "(?:/[^\\\\s]*)?"                                // optional path
            "$",
            std::regex::extended
        );
        
        try {
            // Use regex_match with bounded input to prevent ReDoS
            return std::regex_match(url, urlPattern);
        } catch (const std::regex_error&) {
            // Handle regex errors safely
            return false;
        }
    }
};

int main() {
    URLValidator validator;
    
    // Test cases with various valid and invalid URLs
    std::vector<std::string> testURLs = {
        "https://www.example.com",
        "http://example.com/path/to/resource",
        "ftp://invalid.com",
        "https://sub.domain.example.com:8080/path?query=value",
        ""
    };
    
    std::cout << "URL Validation Results:\\n";
    std::cout << "=======================\\n";
    
    for (const auto& url : testURLs) {
        // Safely handle output with bounded strings
        std::string displayURL = url.empty() ? "(empty string)" : url;
        if (displayURL.length() > 80) {
            displayURL = displayURL.substr(0, 77) + "...";
        }
        
        bool isValid = validator.validate(url);
        std::cout << "URL: " << displayURL << "\\n";
        std::cout << "Valid: " << (isValid ? "YES" : "NO") << "\\n\\n";
    }
    
    return 0;
}
