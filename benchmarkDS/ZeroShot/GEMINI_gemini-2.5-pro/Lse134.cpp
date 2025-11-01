#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <regex>

/**
 * Converts special characters in a string to HTML-safe sequences.
 * This is crucial to prevent Cross-Site Scripting (XSS) attacks.
 *
 * @param input The string to escape.
 * @return The HTML-escaped string.
 */
std::string escape(const std::string& input) {
    std::string escaped;
    escaped.reserve(input.size()); // Reserve memory to avoid reallocations
    for (char c : input) {
        switch (c) {
            case '<':  escaped += "&lt;";   break;
            case '>':  escaped += "&gt;";   break;
            case '&':  escaped += "&amp;";   break;
            case '"':  escaped += "&quot;";  break;
            case '\'': escaped += "&#39;";  break;
            default:   escaped += c;        break;
        }
    }
    return escaped;
}

/**
 * Handler function for the /hello/<username> route.
 *
 * @param username The username extracted from the URL.
 * @return A greeting string with the escaped username.
 */
std::string hello(const std::string& username) {
    return "Hello " + escape(username);
}

class Lse134 {
private:
    // A map to hold URL regex patterns and their corresponding handler functions
    std::map<std::regex, std::function<std::string(const std::smatch&)>> routes;

public:
    /**
     * Simulates a decorator by registering a URL pattern and its handler.
     *
     * @param pathPattern The regex pattern for the URL.
     * @param handler The function to execute when the URL matches.
     */
    void addRoute(const std::string& pathPattern, const std::function<std::string(const std::smatch&)>& handler) {
        routes[std::regex(pathPattern)] = handler;
    }

    /**
     * Handles an incoming URL request by finding a matching route and executing its handler.
     *
     * @param url The URL to handle.
     * @return The response string from the handler, or a 404 message.
     */
    std::string handleRequest(const std::string& url) {
        for (const auto& pair : routes) {
            std::smatch match;
            if (std::regex_match(url, match, pair.first)) {
                // If a match is found, call the handler function with the match results
                return pair.second(match);
            }
        }
        return "404 Not Found";
    }
};

int main() {
    Lse134 router;

    // This simulates the decorator mapping.
    // The pattern uses a capturing group for the username.
    // The lambda function extracts the username from the match and calls the actual 'hello' function.
    router.addRoute("/hello/([^/]+)", [](const std::smatch& match) {
        // match[0] is the entire string, match[1] is the first capture group.
        if (match.size() > 1) {
            return hello(match[1].str());
        }
        return std::string("Error: username not found in URL");
    });
    
    // --- Test Cases ---
    std::cout << "--- CPP Test Cases ---" << std::endl;
    std::vector<std::string> testUrls = {
        "/hello/Alice",
        "/hello/Bob",
        "/hello/<script>alert('xss')</script>", // Security test case
        "/hello/John Doe",
        "/hello/another/path" // Should not match
    };

    for (const auto& url : testUrls) {
        std::cout << "Request URL: " << url << std::endl;
        std::string response = router.handleRequest(url);
        std::cout << "Response: " << response << std::endl << std::endl;
    }

    return 0;
}