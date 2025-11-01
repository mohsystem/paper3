
#include <iostream>
#include <string>
#include <algorithm>
#include <regex>

class Lse31 {
public:
    static std::string redirect_url(const std::string& url) {
        if (url.empty()) {
            return "404 Error: Invalid URL";
        }
        
        // Parse URL using regex to extract domain
        std::regex url_regex(R"(^(?:https?://)?([^:/\\s]+)(?::\\d+)?(?:/.*)?$)", 
                            std::regex::icase);
        std::smatch match;
        
        if (std::regex_match(url, match, url_regex)) {
            std::string domain = match[1].str();
            
            // Convert to lowercase for case-insensitive comparison
            std::transform(domain.begin(), domain.end(), domain.begin(), ::tolower);
            
            // Exact domain match - no subdomains allowed
            if (domain == "example.com") {
                return "Redirect to: " + url;
            } else {
                return "404 Error: Domain not allowed";
            }
        } else {
            return "404 Error: Invalid URL format";
        }
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse31::redirect_url("http://example.com/path") << std::endl;
    std::cout << "Test 2: " << Lse31::redirect_url("https://example.com/secure") << std::endl;
    std::cout << "Test 3: " << Lse31::redirect_url("http://evil.com/malicious") << std::endl;
    std::cout << "Test 4: " << Lse31::redirect_url("http://subdomain.example.com/path") << std::endl;
    std::cout << "Test 5: " << Lse31::redirect_url("invalid-url") << std::endl;
    
    return 0;
}
