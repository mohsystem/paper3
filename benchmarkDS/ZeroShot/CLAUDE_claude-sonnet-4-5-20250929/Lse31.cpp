
#include <iostream>
#include <string>
#include <regex>

class Lse31 {
public:
    static std::string redirectUrl(const std::string& url) {
        try {
            // Extract host from URL using regex
            std::regex urlRegex(R"(^https?://([^/]+)(/.*)?$)");
            std::smatch match;
            
            if (std::regex_search(url, match, urlRegex)) {
                std::string networkLocation = match[1].str();
                
                if (networkLocation == "example.com") {
                    return "Redirect to: " + url;
                } else {
                    return "404 Error: Domain not allowed";
                }
            } else {
                return "404 Error: Invalid URL";
            }
        } catch (...) {
            return "404 Error: Invalid URL";
        }
    }
    
    static void main() {
        // Test cases
        std::string testCases[] = {
            "https://example.com/page1",
            "http://example.com/login",
            "https://notexample.com/page",
            "https://malicious.com/phishing",
            "https://sub.example.com/resource"
        };
        
        for (const auto& testUrl : testCases) {
            std::cout << "URL: " << testUrl << std::endl;
            std::cout << "Result: " << redirectUrl(testUrl) << std::endl;
            std::cout << std::endl;
        }
    }
};

int main() {
    Lse31::main();
    return 0;
}
