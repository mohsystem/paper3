
#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <algorithm>

class Lse134 {
private:
    std::map<std::string, std::function<std::string(std::string)>> routes;
    
    std::string urlDecode(const std::string& str) {
        std::string result;
        char ch;
        int i, ii;
        for (i = 0; i < str.length(); i++) {
            if (str[i] == '%') {
                sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
                ch = static_cast<char>(ii);
                result += ch;
                i = i + 2;
            } else if (str[i] == '+') {
                result += ' ';
            } else {
                result += str[i];
            }
        }
        return result;
    }
    
    std::string escapeHtml(const std::string& input) {
        std::string output;
        for (char c : input) {
            switch (c) {
                case '&':  output += "&amp;"; break;
                case '<':  output += "&lt;"; break;
                case '>':  output += "&gt;"; break;
                case '"':  output += "&quot;"; break;
                case '\\'': output += "&#x27;"; break;
                case '/':  output += "&#x2F;"; break;
                default:   output += c; break;
            }
        }
        return output;
    }
    
public:
    Lse134() {
        routes["/hello/"] = [this](std::string username) { return hello(username); };
    }
    
    std::string hello(const std::string& username) {
        std::string safeUsername = escapeHtml(username);
        return "Hello " + safeUsername;
    }
    
    std::string handleRequest(const std::string& path) {
        if (path.find("/hello/") == 0) {
            std::string username = path.substr(7);
            username = urlDecode(username);
            return hello(username);
        }
        return "Not Found";
    }
};

int main() {
    Lse134 app;
    
    // Test cases
    std::cout << "Test 1: " << app.handleRequest("/hello/John") << std::endl;
    std::cout << "Test 2: " << app.handleRequest("/hello/Alice") << std::endl;
    std::cout << "Test 3: " << app.handleRequest("/hello/<script>alert('XSS')</script>") << std::endl;
    std::cout << "Test 4: " << app.handleRequest("/hello/User&Name") << std::endl;
    std::cout << "Test 5: " << app.handleRequest("/hello/Test%20User") << std::endl;
    
    return 0;
}
