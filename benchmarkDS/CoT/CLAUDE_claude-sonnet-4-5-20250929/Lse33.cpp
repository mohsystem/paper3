
#include <iostream>
#include <string>
#include <algorithm>
#include <regex>

class Lse33 {
public:
    static std::pair<std::string, bool> redirectInvalidUrl(const std::string& urlString) {
        if (urlString.empty()) {
            return {"Error: URL parameter is required", false};
        }
        
        try {
            // Extract host from URL using regex
            std::regex urlRegex(R"(^https?://([^/]+))", std::regex::icase);
            std::smatch match;
            
            if (!std::regex_search(urlString, match, urlRegex)) {
                return {"Error: Invalid URL format", false};
            }
            
            std::string host = match[1].str();
            std::transform(host.begin(), host.end(), host.begin(), ::tolower);
            
            // Strict domain validation - only allow example.com
            if (host == "example.com" || 
                (host.length() > 11 && host.substr(host.length() - 11) == ".example.com")) {
                return {urlString, true};
            } else {
                return {"Error: Invalid domain. Only example.com is allowed", false};
            }
        } catch (const std::exception& e) {
            return {"Error: Invalid URL format", false};
        }
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse33::redirectInvalidUrl("https://example.com/page").first << std::endl;
    std::cout << "Test 2: " << Lse33::redirectInvalidUrl("https://www.example.com/page").first << std::endl;
    std::cout << "Test 3: " << Lse33::redirectInvalidUrl("https://evil.com").first << std::endl;
    std::cout << "Test 4: " << Lse33::redirectInvalidUrl("https://example.com.evil.com").first << std::endl;
    std::cout << "Test 5: " << Lse33::redirectInvalidUrl("invalid-url").first << std::endl;
    
    return 0;
}
