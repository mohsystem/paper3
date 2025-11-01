
#include <iostream>
#include <string>
#include <regex>

class Lse28 {
private:
    // Compile the regular expression once when the class is instantiated
    static const std::regex URL_PATTERN;
    static const std::string DEFAULT_URL;
    
public:
    static std::string getRedirectUrl(const std::string& targetUrl) {
        if (targetUrl.empty()) {
            return DEFAULT_URL;
        }
        
        // If the target URL matches the pattern, use it as redirect target
        if (std::regex_match(targetUrl, URL_PATTERN)) {
            return targetUrl;
        }
        
        // If the target URL does not match, use default URL
        return DEFAULT_URL;
    }
};

// Initialize static members
const std::regex Lse28::URL_PATTERN("^https?://[a-zA-Z0-9.-]+\\\\.[a-zA-Z]{2,}(/.*)?$");
const std::string Lse28::DEFAULT_URL = "https://default.example.com";

int main() {
    // Test cases
    std::string testCases[] = {
        "https://www.example.com/page",
        "http://test.com",
        "invalid-url",
        "ftp://wrong-protocol.com",
        "https://secure-site.org/path/to/resource"
    };
    
    std::cout << "Testing URL Redirect Logic:" << std::endl;
    for (const auto& testUrl : testCases) {
        std::string result = Lse28::getRedirectUrl(testUrl);
        std::cout << "Input: " << testUrl << " -> Redirect: " << result << std::endl;
    }
    
    return 0;
}
