
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>

class Lse132 {
private:
    // HTML escape function for security
    static std::string escapeHtml(const std::string& input) {
        std::string output;
        output.reserve(input.size());
        
        for (char c : input) {
            switch (c) {
                case '&':  output.append("&amp;"); break;
                case '<':  output.append("&lt;"); break;
                case '>':  output.append("&gt;"); break;
                case '"':  output.append("&quot;"); break;
                case '\\'': output.append("&#x27;"); break;
                default:   output.push_back(c); break;
            }
        }
        return output;
    }
    
    // Simulated template rendering
    static std::string renderTemplate(const std::string& templateName, const std::string& username) {
        std::string safeUsername = escapeHtml(username);
        
        std::ostringstream oss;
        oss << "<!DOCTYPE html>\\n"
            << "<html>\\n"
            << "<head>\\n"
            << "    <title>Hello Page</title>\\n"
            << "</head>\\n"
            << "<body>\\n"
            << "    <h1>Hello, " << safeUsername << "!</h1>\\n"
            << "    <p>Welcome to our secure application.</p>\\n"
            << "</body>\\n"
            << "</html>";
        
        return oss.str();
    }

public:
    // Handler function for /hello/<username>
    static std::string hello(const std::string& username) {
        return renderTemplate("hello.html", username);
    }
};

int main() {
    std::cout << "Testing hello function with 5 test cases:\\n\\n";
    
    // Test case 1
    std::cout << "Test 1: Username = 'John'\\n";
    std::cout << Lse132::hello("John") << "\\n\\n";
    
    // Test case 2
    std::cout << "Test 2: Username = 'Alice'\\n";
    std::cout << Lse132::hello("Alice") << "\\n\\n";
    
    // Test case 3
    std::cout << "Test 3: Username = 'Bob123'\\n";
    std::cout << Lse132::hello("Bob123") << "\\n\\n";
    
    // Test case 4
    std::cout << "Test 4: Username = 'User Name'\\n";
    std::cout << Lse132::hello("User Name") << "\\n\\n";
    
    // Test case 5
    std::cout << "Test 5: Username with HTML = '<script>alert(\\"XSS\\")</script>'\\n";
    std::cout << Lse132::hello("<script>alert(\\"XSS\\")</script>") << "\\n\\n";
    
    return 0;
}
