#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <regex>
#include <sstream>

/**
 * @brief Converts special HTML characters to their corresponding entities.
 *
 * This function prevents Cross-Site Scripting (XSS) by escaping characters
 * that have special meaning in HTML.
 *
 * @param input The raw string to escape.
 * @return A new string with HTML characters escaped.
 */
std::string escape(const std::string& input) {
    std::stringstream ss;
    for (char c : input) {
        switch (c) {
            case '&':  ss << "&amp;";       break;
            case '<':  ss << "&lt;";        break;
            case '>':  ss << "&gt;";        break;
            case '"':  ss << "&quot;";      break;
            case '\'': ss << "&#39;";       break;
            default:   ss << c;             break;
        }
    }
    return ss.str();
}

/**
 * @brief The handler function for the /hello/<username> route.
 *
 * @param username The username extracted from the URL.
 * @return A greeting string with the username safely escaped.
 */
std::string hello(const std::string& username) {
    return "Hello " + escape(username);
}

// Router class to simulate URL routing
class Router {
private:
    using Handler = std::function<std::string(const std::smatch&)>;
    std::vector<std::pair<std::regex, Handler>> routes;

public:
    // Simulates a decorator by adding a route and its handler
    void addRoute(const std::string& pattern, Handler handler) {
        routes.emplace_back(std::regex(pattern), handler);
    }

    // Dispatches a URL to the appropriate handler
    std::string handleRequest(const std::string& url) {
        std::smatch match;
        for (const auto& route : routes) {
            if (std::regex_match(url, match, route.first)) {
                return route.second(match);
            }
        }
        return "404 Not Found";
    }
};


int main() {
    Router router;

    // Simulating the decorator: mapping URL pattern to a handler function.
    // The regex captures the username part of the URL.
    router.addRoute("^/hello/(.*)$", [](const std::smatch& match) {
        // match[0] is the whole string, match[1] is the first capture group
        if (match.size() > 1) {
            return hello(match[1].str());
        }
        return hello(""); // Should not happen with this regex, but good practice
    });
    
    // Test cases
    std::cout << "CPP Test Cases:" << std::endl;
    std::vector<std::string> testUrls = {
        "/hello/World",
        "/hello/Jane Doe",
        "/hello/<script>alert('XSS')</script>",
        "/hello/user&'\"<>",
        "/hello/"
    };

    for (const auto& url : testUrls) {
        std::cout << "Request: " << url << std::endl;
        std::cout << "Response: " << router.handleRequest(url) << std::endl;
        std::cout << "---" << std::endl;
    }

    return 0;
}