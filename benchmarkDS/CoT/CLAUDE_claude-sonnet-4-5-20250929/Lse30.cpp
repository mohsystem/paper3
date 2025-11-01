
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

class Lse30 {
private:
    static const std::string ROOT_REDIRECT;
    static const std::regex SAFE_REDIRECT_PATTERN;
    
    static std::string urlDecode(const std::string& str) {
        std::string result;
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == '%' && i + 2 < str.length()) {
                int value;
                std::string hex = str.substr(i + 1, 2);
                std::istringstream iss(hex);
                if (iss >> std::hex >> value) {
                    result += static_cast<char>(value);
                    i += 2;
                } else {
                    result += str[i];
                }
            } else if (str[i] == '+') {
                result += ' ';
            } else {
                result += str[i];
            }
        }
        return result;
    }
    
public:
    static std::string getRedirectUrl(const std::string& target) {
        if (target.empty()) {
            return ROOT_REDIRECT;
        }
        
        try {
            // Decode URL to prevent encoding bypasses
            std::string decodedTarget = urlDecode(target);
            
            // Additional security checks
            // 1. Prevent open redirect by ensuring it's not an absolute URL\n            if (decodedTarget.find("http://") == 0 || \n                decodedTarget.find("https://") == 0 || \n                decodedTarget.find("//") == 0 || \n                decodedTarget.find("@") != std::string::npos) {\n                return ROOT_REDIRECT;\n            }\n            \n            // 2. Check against the regular expression\n            if (std::regex_search(decodedTarget, SAFE_REDIRECT_PATTERN) && \n                std::regex_match(decodedTarget.substr(0, decodedTarget.find('.')+1), SAFE_REDIRECT_PATTERN)) {\n                // Ensure the target doesn't contain path traversal attempts
                if (decodedTarget.find("..") != std::string::npos || 
                    decodedTarget.find("./") != std::string::npos) {
                    return ROOT_REDIRECT;
                }
                return decodedTarget;
            }
            
        } catch (...) {
            // If any error occurs, redirect to root
            return ROOT_REDIRECT;
        }
        
        return ROOT_REDIRECT;
    }
};

const std::string Lse30::ROOT_REDIRECT = "/";
const std::regex Lse30::SAFE_REDIRECT_PATTERN("^[a-zA-Z0-9-]+\\\\.");

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse30::getRedirectUrl("page1.html") << std::endl;
    std::cout << "Test 2: " << Lse30::getRedirectUrl("my-page.html") << std::endl;
    std::cout << "Test 3: " << Lse30::getRedirectUrl("http://evil.com") << std::endl;
    std::cout << "Test 4: " << Lse30::getRedirectUrl("../etc/passwd") << std::endl;
    std::cout << "Test 5: " << Lse30::getRedirectUrl("invalid") << std::endl;
    
    return 0;
}
