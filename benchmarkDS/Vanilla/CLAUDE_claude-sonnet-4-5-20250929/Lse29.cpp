
#include <iostream>
#include <string>
#include <regex>

class Lse29 {
private:
    static const std::regex ALLOWED_PATTERN;
    
public:
    static bool isValidTarget(const std::string& target) {
        if (target.empty()) {
            return false;
        }
        return std::regex_match(target, ALLOWED_PATTERN);
    }
    
    static std::string handleRedirect(const std::string& target) {
        if (!target.empty() && isValidTarget(target)) {
            return target;
        }
        return "/";
    }
};

const std::regex Lse29::ALLOWED_PATTERN(".*example\\\\.com.*");

int main() {
    // Test cases
    std::cout << "Test 1: " << std::boolalpha << Lse29::isValidTarget("https://example.com") 
              << " (Expected: true)" << std::endl;
    std::cout << "Test 2: " << std::boolalpha << Lse29::isValidTarget("https://www.example.com/page") 
              << " (Expected: true)" << std::endl;
    std::cout << "Test 3: " << std::boolalpha << Lse29::isValidTarget("https://subdomain.example.com") 
              << " (Expected: true)" << std::endl;
    std::cout << "Test 4: " << std::boolalpha << Lse29::isValidTarget("https://google.com") 
              << " (Expected: false)" << std::endl;
    std::cout << "Test 5: " << std::boolalpha << Lse29::isValidTarget("https://malicious.com") 
              << " (Expected: false)" << std::endl;
    
    return 0;
}
