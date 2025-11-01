
#include <iostream>
#include <string>
#include <regex>
#include <sstream>
#include <map>
#include <cstring>
#include <algorithm>

// Maximum username length to prevent DoS
const size_t MAX_USERNAME_LENGTH = 50;

// HTML escape function to prevent XSS attacks
std::string escapeHtml(const std::string& input) {
    std::string output;
    output.reserve(input.length() * 2); // Pre-allocate to prevent multiple reallocations
    
    for (char c : input) {
        switch (c) {
            case '&':  output.append("&amp;");  break;
            case '<':  output.append("&lt;");   break;
            case '>':  output.append("&gt;");   break;
            case '"':  output.append("&quot;"); break;
            case '\\'': output.append("&#x27;"); break;
            case '/':  output.append("&#x2F;"); break;
            default:   output.push_back(c);     break;
        }
    }
    
    return output;
}

// Validate username against whitelist pattern
// Only allows alphanumeric characters and underscore
bool isValidUsername(const std::string& username) {
    // Check length constraints
    if (username.empty() || username.length() > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    // Whitelist validation - only alphanumeric and underscore
    std::regex usernamePattern("^[a-zA-Z0-9_]+$");
    return std::regex_match(username, usernamePattern);
}

// Render template with secure variable substitution
std::string renderTemplate(const std::string& username) {
    // Validate username before processing
    if (!isValidUsername(username)) {
        throw std::invalid_argument("Invalid username format");
    }
    
    // Escape username to prevent XSS
    std::string safeUsername = escapeHtml(username);
    
    // Template content
    std::string templateContent = R"(<!DOCTYPE html>\n<html>\n<head>\n    <meta charset="UTF-8">\n    <meta http-equiv="X-Content-Type-Options" content="nosniff">\n    <meta http-equiv="Content-Security-Policy" content="default-src 'self'">\n    <title>Hello</title>\n</head>\n<body>\n    <h1>Hello, {{ username }}!</h1>\n    <p>Welcome to the page.</p>\n</body>\n</html>)";
    
    // Replace placeholder with escaped username
    size_t pos = templateContent.find("{{ username }}");
    if (pos != std::string::npos) {
        templateContent.replace(pos, 14, safeUsername); // 14 is length of "{{ username }}"
    }
    
    return templateContent;
}

// URL route handler simulation
class RouteHandler {
private:
    std::map<std::string, std::string> routes;
    
public:
    // Decorator pattern - bind function to URL pattern
    void route(const std::string& pattern) {
        routes[pattern] = pattern;
    }
    
    // Handle request for /hello/<username>
    std::string handleRequest(const std::string& path) {
        // Extract username from path
        std::regex pathPattern("^/hello/([^/]+)$");
        std::smatch matches;
        
        if (std::regex_match(path, matches, pathPattern)) {
            if (matches.size() == 2) {
                std::string username = matches[1].str();
                
                try {
                    return renderTemplate(username);
                } catch (const std::invalid_argument& e) {
                    // Generic error - don't leak internal details\n                    return "HTTP/1.1 400 Bad Request\\r\\nContent-Type: text/plain\\r\\n\\r\\nInvalid request";\n                }\n            }\n        }\n        \n        return "HTTP/1.1 404 Not Found\\r\\nContent-Type: text/plain\\r\\n\\r\\nNot Found";\n    }\n};\n\nint main() {\n    RouteHandler handler;\n    handler.route("/hello/<username>");\n    \n    // Test Case 1 - Valid alphanumeric username\n    std::cout << "Test Case 1 - Valid alphanumeric username:" << std::endl;\n    try {\n        std::string result = renderTemplate("john123");\n        std::cout << result.substr(0, 100) << "..." << std::endl << std::endl;\n    } catch (const std::exception& e) {\n        std::cout << "Error: " << e.what() << std::endl << std::endl;\n    }\n    \n    // Test Case 2 - Valid username with underscore\n    std::cout << "Test Case 2 - Valid username with underscore:" << std::endl;\n    try {\n        std::string result = renderTemplate("user_name");\n        std::cout << result.substr(0, 100) << "..." << std::endl << std::endl;\n    } catch (const std::exception& e) {\n        std::cout << "Error: " << e.what() << std::endl << std::endl;\n    }\n    \n    // Test Case 3 - Simple username\n    std::cout << "Test Case 3 - Simple username:" << std::endl;\n    try {\n        std::string result = renderTemplate("alice");\n        std::cout << result.substr(0, 100) << "..." << std::endl << std::endl;\n    } catch (const std::exception& e) {\n        std::cout << "Error: " << e.what() << std::endl << std::endl;\n    }\n    \n    // Test Case 4 - XSS attempt (should be rejected)\n    std::cout << "Test Case 4 - XSS attempt (should be rejected):" << std::endl;\n    try {\n        std::string result = renderTemplate("<script>alert('xss')</script>");\n        std::cout << result.substr(0, 100) << "..." << std::endl << std::endl;\n    } catch (const std::exception& e) {\n        std::cout << "Rejected: " << e.what() << std::endl << std::endl;\n    }\n    \n    // Test Case 5 - Path traversal attempt (should be rejected)\n    std::cout << "Test Case 5 - Path traversal attempt (should be rejected):" << std::endl;\n    try {\n        std::string result = renderTemplate("../etc/passwd");\n        std::cout << result.substr(0, 100) << "..." << std::endl << std::endl;\n    } catch (const std::exception& e) {\n        std::cout << "Rejected: " << e.what() << std::endl << std::endl;
    }
    
    return 0;
}
