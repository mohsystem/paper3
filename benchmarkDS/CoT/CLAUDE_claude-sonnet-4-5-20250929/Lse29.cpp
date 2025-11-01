
#include <iostream>
#include <string>
#include <regex>
#include <cstring>

class Lse29 {
private:
    static std::regex SAFE_URL_PATTERN;
    
public:
    static bool isValidTarget(const std::string& target) {
        if (target.empty()) {
            return false;
        }
        return std::regex_match(target, SAFE_URL_PATTERN);
    }
    
    static std::string getRedirectTarget(const std::string& target) {
        if (isValidTarget(target)) {
            return target;
        }
        return "/";
    }
    
    static void runTests() {
        std::cout << "Test Cases:" << std::endl;
        std::cout << "1. Valid: http://example.com -> " 
                  << isValidTarget("http://example.com") << std::endl;
        std::cout << "2. Valid: https://www.example.com/path -> " 
                  << isValidTarget("https://www.example.com/path") << std::endl;
        std::cout << "3. Invalid: http://malicious.com -> " 
                  << isValidTarget("http://malicious.com") << std::endl;
        std::cout << "4. Invalid: javascript:alert(1) -> " 
                  << isValidTarget("javascript:alert(1)") << std::endl;
        std::cout << "5. Valid: http://subdomain.example.com -> " 
                  << isValidTarget("http://subdomain.example.com") << std::endl;
    }
};

std::regex Lse29::SAFE_URL_PATTERN(R"(^https?://([a-zA-Z0-9-]+\\.)*example\\.com(/.*)?$)");

int main() {
    Lse29::runTests();
    return 0;
}
