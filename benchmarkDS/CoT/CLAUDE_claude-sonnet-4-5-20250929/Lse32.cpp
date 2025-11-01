
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

class Lse32 {
public:
    static std::string redirectInvalidUrl(const std::string& urlString) {
        if (urlString.empty()) {
            return "Error: URL cannot be empty";
        }
        
        // Parse URL using regex
        std::regex urlRegex(R"(^(https?):\\/\\/([^\\/]+)(\\/.*)?$)", std::regex::icase);
        std::smatch match;
        
        if (!std::regex_match(urlString, match, urlRegex)) {
            return "Error: Invalid URL format";
        }
        
        std::string protocol = match[1].str();
        std::string host = match[2].str();
        
        // Convert host to lowercase
        std::transform(host.begin(), host.end(), host.begin(), ::tolower);
        
        // Remove port if present
        size_t colonPos = host.find(':');
        if (colonPos != std::string::npos) {
            host = host.substr(0, colonPos);
        }
        
        // Validate protocol
        if (protocol != "http" && protocol != "https") {
            return "Error: Invalid protocol. Only http and https are allowed";
        }
        
        // Validate domain
        if (host != "example.com" && 
            (host.length() <= 11 || host.substr(host.length() - 11) != ".example.com")) {
            return "Error: URL must be from example.com domain";
        }
        
        return "Redirect to: " + urlString;
    }
};

int main() {
    // Test cases
    std::cout << "Test Case 1:\\n" << Lse32::redirectInvalidUrl("https://example.com/page") << "\\n\\n";
    std::cout << "Test Case 2:\\n" << Lse32::redirectInvalidUrl("https://sub.example.com/path") << "\\n\\n";
    std::cout << "Test Case 3:\\n" << Lse32::redirectInvalidUrl("https://evil.com") << "\\n\\n";
    std::cout << "Test Case 4:\\n" << Lse32::redirectInvalidUrl("https://example.com.evil.com") << "\\n\\n";
    std::cout << "Test Case 5:\\n" << Lse32::redirectInvalidUrl("ftp://example.com") << "\\n";
    
    return 0;
}
