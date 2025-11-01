
#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <cctype>

// Simple URL parser for validation
struct ParsedURL {
    std::string scheme;
    std::string host;
    std::string path;
    bool valid;
    
    ParsedURL() : valid(false) {}
};

// Parse URL into components
ParsedURL parseURL(const std::string& url) {
    ParsedURL result;
    
    if (url.empty()) {
        return result;
    }
    
    // Find scheme
    size_t schemeEnd = url.find("://");
    if (schemeEnd == std::string::npos) {
        return result;
    }
    
    result.scheme = url.substr(0, schemeEnd);
    
    // Convert scheme to lowercase
    std::transform(result.scheme.begin(), result.scheme.end(), 
                   result.scheme.begin(), ::tolower);
    
    // Find host start
    size_t hostStart = schemeEnd + 3;
    size_t hostEnd = url.find('/', hostStart);
    
    if (hostEnd == std::string::npos) {
        result.host = url.substr(hostStart);
        result.path = "/";
    } else {
        result.host = url.substr(hostStart, hostEnd - hostStart);
        result.path = url.substr(hostEnd);
    }
    
    // Remove port if present
    size_t portPos = result.host.find(':');
    if (portPos != std::string::npos) {
        result.host = result.host.substr(0, portPos);
    }
    
    // Convert host to lowercase
    std::transform(result.host.begin(), result.host.end(), 
                   result.host.begin(), ::tolower);
    
    result.valid = !result.scheme.empty() && !result.host.empty();
    return result;
}

// Validates if the URL is from example.com domain
bool isValidExampleUrl(const std::string& urlString) {
    if (urlString.empty()) {
        return false;
    }
    
    // Parse the URL
    ParsedURL parsed = parseURL(urlString);
    
    if (!parsed.valid) {
        return false;
    }
    
    // Only allow https for security
    if (parsed.scheme != "https") {
        return false;
    }
    
    // Check if host is exactly example.com or subdomain of example.com
    // Prevent bypass attempts like "example.com.evil.com"
    if (parsed.host == "example.com") {
        return true;
    }
    
    // Check if it ends with .example.com
    const std::string suffix = ".example.com";
    if (parsed.host.length() > suffix.length()) {
        size_t pos = parsed.host.length() - suffix.length();
        if (parsed.host.substr(pos) == suffix) {
            return true;
        }
    }
    
    return false;
}

// Simulated redirect function that would be called by web framework
std::string redirectInvalidUrl(const std::string& url) {
    if (isValidExampleUrl(url)) {
        return "REDIRECT: " + url;
    } else {
        return "ERROR: Invalid URL. Only example.com URLs are allowed.";
    }
}

int main() {
    // Test cases
    std::cout << "Test Case 1 - Valid example.com URL:\\n";
    std::string test1 = "https://example.com/page";
    std::cout << "URL: " << test1 << "\\n";
    std::cout << "Result: " << redirectInvalidUrl(test1) << "\\n\\n";
    
    std::cout << "Test Case 2 - Valid subdomain:\\n";
    std::string test2 = "https://sub.example.com/path";
    std::cout << "URL: " << test2 << "\\n";
    std::cout << "Result: " << redirectInvalidUrl(test2) << "\\n\\n";
    
    std::cout << "Test Case 3 - Invalid domain:\\n";
    std::string test3 = "https://evil.com";
    std::cout << "URL: " << test3 << "\\n";
    std::cout << "Result: " << redirectInvalidUrl(test3) << "\\n\\n";
    
    std::cout << "Test Case 4 - Bypass attempt:\\n";
    std::string test4 = "https://example.com.evil.com";
    std::cout << "URL: " << test4 << "\\n";
    std::cout << "Result: " << redirectInvalidUrl(test4) << "\\n\\n";
    
    std::cout << "Test Case 5 - HTTP not allowed:\\n";
    std::string test5 = "http://example.com";
    std::cout << "URL: " << test5 << "\\n";
    std::cout << "Result: " << redirectInvalidUrl(test5) << "\\n";
    
    return 0;
}
