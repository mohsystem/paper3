#include <iostream>
#include <string>
#include <map>
#include <functional>
#include <regex>

// A simple HTML escape function to prevent XSS.
std::string escape(const std::string& input) {
    std::string output;
    output.reserve(input.length()); // Pre-allocate memory
    for (char c : input) {
        switch (c) {
            case '&':  output += "&amp;";       break;
            case '\"': output += "&quot;";      break;
            case '\'': output += "&#x27;";      break;
            case '<':  output += "&lt;";        break;
            case '>':  output += "&gt;";        break;
            default:   output += c;             break;
        }
    }
    return output;
}

// Handler function for the /hello/<username> route.
std::string hello(const std::string& username) {
    // Rule #1: Sanitize input before processing.
    std::string escaped_username = escape(username);
    return "Hello, " + escaped_username + "!";
}

// A simple router class to simulate URL mapping.
class Router {
public:
    // Maps a regex pattern to a handler function. Simulates a decorator.
    void add_route(const std::string& pattern, std::function<std::string(const std::string&)> handler) {
        routes_[pattern] = handler;
    }

    // Dispatches a request to the appropriate handler.
    std::string dispatch(const std::string& url) {
        for (const auto& pair : routes_) {
            try {
                std::regex rgx(pair.first);
                std::smatch match;
                if (std::regex_match(url, match, rgx) && match.size() > 1) {
                    return pair.second(match[1].str());
                }
            } catch (const std::regex_error& e) {
                // Handle potential regex compilation errors
                std::cerr << "Regex error: " << e.what() << std::endl;
                return "500 Internal Server Error";
            }
        }
        return "404 Not Found";
    }

private:
    std::map<std::string, std::function<std::string(const std::string&)>> routes_;
};

int main() {
    Router router;

    // This simulates a decorator creating a mapping for the URL.
    router.add_route("/hello/(.*)", hello);

    // --- 5 Test Cases ---
    std::string test_urls[] = {
        "/hello/World",
        "/hello/CPP",
        "/hello/<script>alert('xss')</script>",
        "/hello/O'Malley",
        "/hello/a&b"
    };

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& url : test_urls) {
        std::string response = router.dispatch(url);
        std::cout << "Request URL: " << url << std::endl;
        std::cout << "Response: " << response << std::endl;
        std::cout << std::endl;
    }

    return 0;
}