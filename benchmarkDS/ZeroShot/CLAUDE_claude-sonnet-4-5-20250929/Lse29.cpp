
#include <iostream>
#include <string>
#include <regex>

class Lse29 {
private:
    static std::regex allowedUrlPattern;
    
public:
    static bool isValidTarget(const std::string& target) {
        if (target.empty()) {
            return false;
        }
        return std::regex_match(target, allowedUrlPattern);
    }
    
    static std::string getRedirectUrl(const std::string& target) {
        if (isValidTarget(target)) {
            return target;
        }
        return "/";
    }
};

std::regex Lse29::allowedUrlPattern("^https?://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/.*)?$");

int main() {
    // Test cases
    std::string testCases[] = {
        "http://example.com",
        "https://subdomain.example.com/path",
        "http://malicious.com",
        "https://example.com.malicious.com",
        ""
    };
    
    std::cout << "Test Results:" << std::endl;
    for (int i = 0; i < 5; i++) {
        std::string result = Lse29::getRedirectUrl(testCases[i]);
        std::cout << "Test Case " << (i + 1) << ": Input: \\"" 
                  << testCases[i] << "\\" -> Output: \\"" << result << "\\"" << std::endl;
    }
    
    return 0;
}
