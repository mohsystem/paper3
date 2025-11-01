
#include <iostream>
#include <string>
#include <regex>

class Lse30 {
public:
    static std::string secureRedirect(const std::string& target) {
        if (target.empty()) {
            return "/";
        }
        
        std::regex redirectPattern("^[a-zA-Z0-9-]+\\\\.");
        
        if (std::regex_search(target, redirectPattern)) {
            return target;
        } else {
            return "/";
        }
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << Lse30::secureRedirect("example.com") << std::endl;  // Should return "example.com"
    std::cout << "Test 2: " << Lse30::secureRedirect("my-site.org") << std::endl;  // Should return "my-site.org"
    std::cout << "Test 3: " << Lse30::secureRedirect("test123.net") << std::endl;  // Should return "test123.net"
    std::cout << "Test 4: " << Lse30::secureRedirect(".invalid") << std::endl;     // Should return "/"
    std::cout << "Test 5: " << Lse30::secureRedirect("no-period") << std::endl;    // Should return "/"
    
    return 0;
}
