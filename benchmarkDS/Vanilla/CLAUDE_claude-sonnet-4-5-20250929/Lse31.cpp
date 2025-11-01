
#include <iostream>
#include <string>
#include <regex>

class Lse31 {
public:
    static std::string redirect_url(const std::string& url) {
        std::regex urlRegex("^https?://([^/:]+)");
        std::smatch match;
        
        if (std::regex_search(url, match, urlRegex)) {
            std::string networkLocation = match[1].str();
            
            if (networkLocation == "example.com") {
                return "Redirect to: " + url;
            } else {
                return "404 Error: Domain not allowed";
            }
        }
        
        return "404 Error: Invalid URL";
    }
};

int main() {
    // Test cases
    std::cout << Lse31::redirect_url("http://example.com/page") << std::endl;
    std::cout << Lse31::redirect_url("https://example.com/admin") << std::endl;
    std::cout << Lse31::redirect_url("http://notexample.com/page") << std::endl;
    std::cout << Lse31::redirect_url("https://evil.com/phishing") << std::endl;
    std::cout << Lse31::redirect_url("http://example.com:8080/secure") << std::endl;
    
    return 0;
}
