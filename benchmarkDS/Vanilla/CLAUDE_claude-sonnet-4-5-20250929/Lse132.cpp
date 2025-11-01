
// C++ using cpp-httplib library (simple HTTP server)
// Note: This requires the cpp-httplib header-only library
#include <iostream>
#include <string>
#include <sstream>

// Simulated HTTP server functionality (actual implementation would require a web framework)
// This is a simplified demonstration as C++ doesn't have built-in web decorators

class Lse132 {
public:
    static std::string hello(const std::string& username) {
        std::stringstream html;
        html << "<!DOCTYPE html>\\n"
             << "<html>\\n"
             << "<head><title>Hello</title></head>\\n"
             << "<body>\\n"
             << "    <h1>Hello, " << username << "!</h1>\\n"
             << "</body>\\n"
             << "</html>";
        return html.str();
    }
};

int main() {
    // Test cases (demonstration of the function logic)
    std::cout << "Test Case 1: /hello/John\\n";
    std::cout << Lse132::hello("John") << "\\n\\n";
    
    std::cout << "Test Case 2: /hello/Alice\\n";
    std::cout << Lse132::hello("Alice") << "\\n\\n";
    
    std::cout << "Test Case 3: /hello/Bob\\n";
    std::cout << Lse132::hello("Bob") << "\\n\\n";
    
    std::cout << "Test Case 4: /hello/Charlie\\n";
    std::cout << Lse132::hello("Charlie") << "\\n\\n";
    
    std::cout << "Test Case 5: /hello/Diana\\n";
    std::cout << Lse132::hello("Diana") << "\\n\\n";
    
    // Note: Full web server implementation would require libraries like:
    // cpp-httplib, Boost.Beast, or Crow framework
    
    return 0;
}
