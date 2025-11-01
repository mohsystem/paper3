
#include <iostream>
#include <string>
#include <algorithm>
#include <cstring>
#include <regex>

// Simple URL validation function for example.com domain
// Returns true only if the URL is valid and from example.com
bool isValidExampleComUrl(const std::string& urlString) {
    if (urlString.empty()) {
        return false;
    }
    
    // Check for valid scheme (http or https)
    if (urlString.find("http://") != 0 && urlString.find("https://") != 0) {
        return false;
    }
    
    // Extract the host part (between :// and first / or end)
    size_t schemeEnd = urlString.find("://");
    if (schemeEnd == std::string::npos) {
        return false;
    }
    
    size_t hostStart = schemeEnd + 3;
    size_t pathStart = urlString.find('/', hostStart);
    size_t hostEnd = (pathStart != std::string::npos) ? pathStart : urlString.length();
    
    std::string netloc = urlString.substr(hostStart, hostEnd - hostStart);
    
    // Check for @ symbol (prevents user info injection)
    if (netloc.find('@') != std::string::npos) {
        return false;
    }
    
    // Remove port if present
    size_t portPos = netloc.find(':');
    std::string host = (portPos != std::string::npos) ? netloc.substr(0, portPos) : netloc;
    
    // Convert to lowercase for comparison
    std::transform(host.begin(), host.end(), host.begin(), ::tolower);
    
    // Only allow exact match of example.com or subdomains of example.com
    // This prevents attacks like example.com.evil.com
    if (host != "example.com") {
        // Check if it's a subdomain
        if (host.length() <= 12) { // ".example.com" is 12 chars
            return false;
        }
        std::string suffix = host.substr(host.length() - 12);
        if (suffix != ".example.com") {
            return false;
        }
    }
    
    return true;
}

// Simulated redirect function
// In a real web application, this would set HTTP redirect headers
std::string redirectInvalidUrl(const std::string& url) {
    // Validate input is not empty
    if (url.empty()) {
        return "Error: URL parameter is required";
    }
    
    // Validate the URL is from example.com domain
    if (!isValidExampleComUrl(url)) {
        return "Error: Invalid URL. Only example.com URLs are allowed.";
    }
    
    // In a real web app, this would set Location header and return 302
    return "Redirect to: " + url;
}

int main() {
    // Test cases
    std::cout << "Test Case 1 - Valid example.com URL:" << std::endl;
    std::string test1 = "https://example.com/path";
    std::cout << (isValidExampleComUrl(test1) ? "PASS" : "FAIL") << std::endl;
    std::cout << redirectInvalidUrl(test1) << std::endl << std::endl;
    
    std::cout << "Test Case 2 - Valid subdomain:" << std::endl;
    std::string test2 = "https://sub.example.com/page";
    std::cout << (isValidExampleComUrl(test2) ? "PASS" : "FAIL") << std::endl;
    std::cout << redirectInvalidUrl(test2) << std::endl << std::endl;
    
    std::cout << "Test Case 3 - Invalid domain:" << std::endl;
    std::string test3 = "https://evil.com";
    std::cout << (!isValidExampleComUrl(test3) ? "PASS" : "FAIL") << std::endl;
    std::cout << redirectInvalidUrl(test3) << std::endl << std::endl;
    
    std::cout << "Test Case 4 - Domain confusion attack:" << std::endl;
    std::string test4 = "https://example.com.evil.com";
    std::cout << (!isValidExampleComUrl(test4) ? "PASS" : "FAIL") << std::endl;
    std::cout << redirectInvalidUrl(test4) << std::endl << std::endl;
    
    std::cout << "Test Case 5 - Invalid scheme:" << std::endl;
    std::string test5 = "javascript:alert(1)";
    std::cout << (!isValidExampleComUrl(test5) ? "PASS" : "FAIL") << std::endl;
    std::cout << redirectInvalidUrl(test5) << std::endl << std::endl;
    
    return 0;
}
