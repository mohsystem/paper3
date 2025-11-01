
#include <iostream>
#include <string>
#include <sstream>
#include <map>

class Lse134 {
public:
    // HTML escape function to prevent XSS attacks
    static std::string escape(const std::string& input) {
        std::stringstream escaped;
        for (char c : input) {
            switch (c) {
                case '<':
                    escaped << "&lt;";
                    break;
                case '>':
                    escaped << "&gt;";
                    break;
                case '&':
                    escaped << "&amp;";
                    break;
                case '"':
                    escaped << "&quot;";
                    break;
                case '\\'':
                    escaped << "&#x27;";
                    break;
                case '/':
                    escaped << "&#x2F;";
                    break;
                default:
                    escaped << c;
            }
        }
        return escaped.str();
    }
    
    // Hello function that returns greeting with escaped username
    static std::string hello(const std::string& username) {
        std::string sanitizedUsername = escape(username);
        return "Hello " + sanitizedUsername;
    }
};

int main() {
    // Test cases
    std::cout << "Test Cases:" << std::endl;
    std::cout << "1. " << Lse134::hello("John") << std::endl;
    std::cout << "2. " << Lse134::hello("Alice<script>alert('XSS')</script>") << std::endl;
    std::cout << "3. " << Lse134::hello("Bob&Carol") << std::endl;
    std::cout << "4. " << Lse134::hello("User\\"123\\"") << std::endl;
    std::cout << "5. " << Lse134::hello("Test'User") << std::endl;
    
    // Note: C++ doesn't have built-in web framework decorators like Python's Flask
    // This demonstrates the core logic for HTML escaping and hello function
    std::cout << "\\nNote: C++ implementation shows core logic." << std::endl;
    std::cout << "For web routing, use frameworks like Crow, Pistache, or cpp-httplib" << std::endl;
    
    return 0;
}
