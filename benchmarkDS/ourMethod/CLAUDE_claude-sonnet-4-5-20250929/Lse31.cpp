
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>

/**
 * Simple URL parser to extract domain from URL
 * Validates scheme and extracts host for security
 */
class URLParser {
private:
    std::string scheme;
    std::string host;
    
public:
    bool parse(const std::string& url) {
        // Input validation: check for null or empty
        if (url.empty()) {
            return false;
        }
        
        // Find scheme separator
        size_t schemeEnd = url.find("://");
        if (schemeEnd == std::string::npos) {
            return false;
        }
        
        scheme = url.substr(0, schemeEnd);
        
        // Extract host starting after ://
        size_t hostStart = schemeEnd + 3;
        size_t hostEnd = url.find_first_of("/?#", hostStart);
        
        if (hostEnd == std::string::npos) {
            host = url.substr(hostStart);
        } else {
            host = url.substr(hostStart, hostEnd - hostStart);
        }
        
        // Normalize to lowercase - prevent bypass via case variation
        std::transform(scheme.begin(), scheme.end(), scheme.begin(), ::tolower);
        std::transform(host.begin(), host.end(), host.begin(), ::tolower);
        
        return true;
    }
    
    std::string getHost() const { return host; }
    std::string getScheme() const { return scheme; }
};

/**
 * Redirects to the URL if domain is example.com, otherwise returns 404.
 * 
 * @param url The URL to validate and redirect
 * @return Redirect message or 404 error
 */
std::string redirectUrl(const std::string& url) {
    // Input validation: reject empty URLs
    if (url.empty()) {
        return "404 - Not Found";
    }
    
    // Limit URL length to prevent resource exhaustion
    if (url.length() > 2048) {
        return "404 - Not Found";
    }
    
    URLParser parser;
    
    // Parse the URL safely - no code execution risk
    if (!parser.parse(url)) {
        return "404 - Not Found";
    }
    
    std::string domain = parser.getHost();
    std::string scheme = parser.getScheme();
    
    // Validate domain is present - fail closed
    if (domain.empty()) {
        return "404 - Not Found";
    }
    
    // Whitelist exact match only - prevent subdomain attacks
    if (domain == "example.com") {
        // Only redirect if scheme is http or https - prevent javascript: data: etc
        if (scheme == "http" || scheme == "https") {
            return "Redirect to: " + url;
        }
    }
    
    // Fail closed - reject anything not explicitly allowed
    return "404 - Not Found";
}

int main() {
    // Test cases
    std::string testUrls[] = {
        "http://example.com/path",
        "https://example.com/secure",
        "http://notexample.com/path",
        "http://subdomain.example.com/path",
        "javascript:alert(1)"
    };
    
    for (const auto& testUrl : testUrls) {
        std::cout << "URL: " << testUrl << std::endl;
        std::cout << "Result: " << redirectUrl(testUrl) << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
