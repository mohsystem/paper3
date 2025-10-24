
#include <iostream>
#include <string>
#include <regex>

class Task110 {
public:
    static bool validateURL(const std::string& url) {
        if (url.empty()) {
            return false;
        }
        
        // Regular expression pattern for URL validation
        std::string urlPattern = "^(https?|ftp)://"  // Protocol
                                "("
                                "([a-zA-Z0-9-]+\\\\.)+[a-zA-Z]{2,}"  // Domain name
                                "|"
                                "localhost"  // OR localhost
                                "|"
                                "((\\\\d{1,3}\\\\.){3}\\\\d{1,3})"  // OR IP address
                                ")"
                                "(:\\\\d+)?"  // Optional port
                                "(/[a-zA-Z0-9._~:/?#\\\\[\\\\]@!$&'()*+,;=-]*)?"  // Optional path
                                "$";
        
        std::regex pattern(urlPattern);
        return std::regex_match(url, pattern);
    }
};

int main() {
    // Test cases
    std::string testUrls[] = {
        "https://www.example.com",
        "http://localhost:8080/path/to/resource",
        "ftp://192.168.1.1/files",
        "invalid-url-without-protocol",
        "https://subdomain.example.co.uk:3000/api?param=value"
    };
    
    std::cout << "URL Validation Results:" << std::endl;
    for (const auto& url : testUrls) {
        bool isValid = Task110::validateURL(url);
        std::cout << url << " -> " << (isValid ? "Valid" : "Invalid") << std::endl;
    }
    
    return 0;
}
