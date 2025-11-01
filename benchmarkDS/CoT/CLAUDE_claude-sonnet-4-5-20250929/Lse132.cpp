
// C++ - Using Crow Framework (Header-only C++ web framework)
#include <iostream>
#include <string>
#include <regex>
#include <algorithm>

class Lse132 {
private:
    static std::string htmlEscape(const std::string& data) {
        std::string escaped;
        escaped.reserve(data.size());
        for (char c : data) {
            switch (c) {
                case '&':  escaped.append("&amp;"); break;
                case '\\"': escaped.append("&quot;"); break;
                case '\\'': escaped.append("&#x27;"); break;
                case '<':  escaped.append("&lt;"); break;
                case '>':  escaped.append("&gt;"); break;
                default:   escaped.push_back(c); break;
            }
        }
        return escaped;
    }
    
    static bool isValidUsername(const std::string& username) {
        if (username.empty() || username.length() > 50) {
            return false;
        }
        std::regex pattern("^[a-zA-Z0-9_-]+$");
        return std::regex_match(username, pattern);
    }
    
    static std::string renderTemplate(const std::string& username, const std::string& error = "") {
        std::string html = "<!DOCTYPE html><html><head><title>Hello</title></head><body>";
        html += "<h1>Hello, " + username + "!</h1>";
        if (!error.empty()) {
            html += "<p style=\\"color: red;\\">" + error + "</p>";
        }
        html += "</body></html>";
        return html;
    }

public:
    static std::string hello(const std::string& username) {
        // Input validation
        if (!isValidUsername(username)) {
            return renderTemplate("Invalid User", 
                "Username must be alphanumeric (1-50 chars)");
        }
        
        // Sanitize input
        std::string sanitizedUsername = htmlEscape(username);
        return renderTemplate(sanitizedUsername);
    }
    
    static void runTests() {
        std::cout << "Test cases for /hello/<username>:" << std::endl;
        
        std::string testCases[] = {
            "john",
            "jane_doe",
            "user-123",
            "<script>alert('xss')</script>",
            "verylongusernamethatshouldbevalidatedproperly"
        };
        
        for (int i = 0; i < 5; i++) {
            std::cout << (i + 1) << ". /hello/" << testCases[i] << std::endl;
            std::string result = hello(testCases[i]);
            std::cout << "   Result length: " << result.length() << " bytes" << std::endl;
        }
    }
};

int main() {
    Lse132::runTests();
    return 0;
}
