#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <sstream>

// The function escape() converts special characters to HTML-safe sequences.
std::string escape(const std::string& str) {
    std::stringstream ss;
    for (char c : str) {
        switch (c) {
            case '<':  ss << "&lt;";   break;
            case '>':  ss << "&gt;";   break;
            case '&':  ss << "&amp;";  break;
            case '"':  ss << "&quot;"; break;
            case '\'': ss << "&#39;";  break;
            default:   ss << c;
        }
    }
    return ss.str();
}

// The function hello() returns a string with the text Hello followed by the value of the variable username.
std::string hello(const std::string& username) {
    return "Hello, " + escape(username) + "!";
}

// Simulates a router that handles requests.
class Router {
public:
    // A mapping between the URL given as an argument and the function that follows it.
    std::map<std::string, std::function<std::string(const std::string&)>> routes;

    void add_route(const std::string& prefix, std::function<std::string(const std::string&)> handler) {
        routes[prefix] = handler;
    }

    std::string handle_request(const std::string& url) {
        for (const auto& pair : routes) {
            const std::string& prefix = pair.first;
            if (url.rfind(prefix, 0) == 0) { // Check if url starts with prefix
                const std::string& param = url.substr(prefix.length());
                return pair.second(param); // Call the mapped function
            }
        }
        return "404 Not Found";
    }
};

int main() {
    Router router;
    
    // The decorator creates a mapping... here we do it manually.
    router.add_route("/hello/", hello);

    // --- 5 Test Cases ---
    std::cout << "Test Case 1: Simple username" << std::endl;
    std::cout << "Input: /hello/Alice" << std::endl;
    std::cout << "Output: " << router.handle_request("/hello/Alice") << std::endl << std::endl;

    std::cout << "Test Case 2: Username with HTML characters" << std::endl;
    std::cout << "Input: /hello/<script>alert('XSS')</script>" << std::endl;
    std::cout << "Output: " << router.handle_request("/hello/<script>alert('XSS')</script>") << std::endl << std::endl;

    std::cout << "Test Case 3: Empty username" << std::endl;
    std::cout << "Input: /hello/" << std::endl;
    std::cout << "Output: " << router.handle_request("/hello/") << std::endl << std::endl;

    std::cout << "Test Case 4: Mismatched URL" << std::endl;
    std::cout << "Input: /goodbye/Bob" << std::endl;
    std::cout << "Output: " << router.handle_request("/goodbye/Bob") << std::endl << std::endl;
    
    std::cout << "Test Case 5: Username with various special characters" << std::endl;
    std::cout << "Input: /hello/Tom & Jerry > \"The Movie\"" << std::endl;
    std::cout << "Output: " << router.handle_request("/hello/Tom & Jerry > \"The Movie\"") << std::endl << std::endl;

    return 0;
}