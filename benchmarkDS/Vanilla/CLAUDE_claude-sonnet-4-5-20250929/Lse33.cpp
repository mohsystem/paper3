
#include <iostream>
#include <string>
#include <regex>

class Lse33 {
public:
    static std::string redirect_invalid_url(const std::string& url_string) {
        try {
            std::regex url_regex(R"(^https?://([^/]+)(/.*)?$)");
            std::smatch matches;
            
            if (std::regex_search(url_string, matches, url_regex)) {
                std::string host = matches[1].str();
                
                if (host == "example.com") {
                    return "Redirect to: " + url_string;
                } else {
                    return "Error: Invalid domain. Only example.com is allowed.";
                }
            } else {
                return "Error: Invalid URL format.";
            }
        } catch (const std::exception& e) {
            return "Error: Invalid URL format.";
        }
    }
};

int main() {
    // Test cases
    std::string test_cases[] = {
        "http://example.com/page1",
        "https://example.com/secure",
        "http://malicious.com/page",
        "http://subdomain.example.com/test",
        "invalid-url-format"
    };
    
    for (const auto& test_case : test_cases) {
        std::cout << "Input: " << test_case << std::endl;
        std::cout << "Output: " << Lse33::redirect_invalid_url(test_case) << std::endl;
        std::cout << std::endl;
    }
    
    return 0;
}
