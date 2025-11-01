
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <map>
#include <functional>

// Whitelist pattern for username validation
const std::regex VALID_USERNAME("^[a-zA-Z0-9_-]{1,50}$");

// HTML escape function to prevent XSS
std::string escape(const std::string& input) {
    // Validate input length to prevent DoS
    if (input.empty() || input.length() > 1000) {
        return "";
    }
    
    std::string escaped;
    escaped.reserve(input.length() * 2); // Pre-allocate to prevent frequent reallocation
    
    for (char c : input) {
        switch (c) {
            case '<':
                escaped.append("&lt;");
                break;
            case '>':
                escaped.append("&gt;");
                break;
            case '&':
                escaped.append("&amp;");
                break;
            case '"':
                escaped.append("&quot;");
                break;
            case '\\'':
                escaped.append("&#x27;");
                break;
            case '/':
                escaped.append("&#x2F;");
                break;
            default:
                escaped.push_back(c);
        }
    }
    
    return escaped;
}

// Hello function that returns greeting with escaped username
std::string hello(const std::string& username) {
    // Input validation - reject empty
    if (username.empty()) {
        return "Hello Guest";
    }
    
    // Validate username against whitelist pattern
    if (!std::regex_match(username, VALID_USERNAME)) {
        return "Hello Guest";
    }
    
    // Escape the username to prevent XSS
    std::string escapedUsername = escape(username);
    return "Hello " + escapedUsername;
}

// Simple route mapping simulation
class Router {
private:
    std::map<std::string, std::function<std::string(const std::string&)>> routes;
    
public:
    void addRoute(const std::string& pattern, std::function<std::string(const std::string&)> handler) {
        routes[pattern] = handler;
    }
    
    std::string handleRequest(const std::string& path, const std::string& param) {
        auto it = routes.find(path);
        if (it != routes.end()) {
            return it->second(param);
        }
        return "Not Found";
    }
};

int main() {
    // Test cases
    std::cout << "Test 1: " << hello("john") << std::endl;
    std::cout << "Test 2: " << hello("alice_123") << std::endl;
    std::cout << "Test 3: " << hello("<script>alert('xss')</script>") << std::endl;
    std::cout << "Test 4: " << hello("user-name") << std::endl;
    std::cout << "Test 5: " << hello("") << std::endl;
    
    // Simulate routing
    Router router;
    
    // Add route with decorator pattern simulation
    router.addRoute("/hello/", [](const std::string& username) -> std::string {
        // Additional validation on route parameter
        if (username.length() > 50) {
            return "Hello Guest";
        }
        return hello(username);
    });
    
    // Simulate requests
    std::cout << "\\nRoute Test 1: " << router.handleRequest("/hello/", "john") << std::endl;
    std::cout << "Route Test 2: " << router.handleRequest("/hello/", "alice_123") << std::endl;
    
    return 0;
}
