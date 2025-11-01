
#include <iostream>
#include <string>
#include <algorithm>

class Lse32 {
public:
    static std::string redirectInvalidUrl(const std::string& url) {
        std::string processedUrl = url;
        
        if (processedUrl.find("http://") != 0 && processedUrl.find("https://") != 0) {
            processedUrl = "http://" + processedUrl;
        }
        
        size_t hostStart = processedUrl.find("://");
        if (hostStart == std::string::npos) {
            return "Error: Invalid URL format";
        }
        
        hostStart += 3;
        size_t hostEnd = processedUrl.find("/", hostStart);
        std::string host;
        
        if (hostEnd != std::string::npos) {
            host = processedUrl.substr(hostStart, hostEnd - hostStart);
        } else {
            host = processedUrl.substr(hostStart);
        }
        
        if (host == "example.com" || 
            (host.length() > 11 && host.substr(host.length() - 11) == ".example.com")) {
            return processedUrl;
        } else {
            return "Error: Invalid URL. Only example.com URLs are allowed.";
        }
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse32::redirectInvalidUrl("example.com/page") << std::endl;
    std::cout << "Test 2: " << Lse32::redirectInvalidUrl("http://example.com/test") << std::endl;
    std::cout << "Test 3: " << Lse32::redirectInvalidUrl("https://www.example.com/home") << std::endl;
    std::cout << "Test 4: " << Lse32::redirectInvalidUrl("google.com") << std::endl;
    std::cout << "Test 5: " << Lse32::redirectInvalidUrl("http://malicious.com") << std::endl;
    
    return 0;
}
