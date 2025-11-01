#include <iostream>
#include <string>
#include <vector>
#include <regex>

// The content of the "hello.html" template file.
const std::string HELLO_TEMPLATE =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "    <title>Hello Page</title>\n"
    "</head>\n"
    "<body>\n"
    "    <h1>Hello, {{username}}!</h1>\n"
    "</body>\n"
    "</html>";

/**
 * Escapes special HTML characters to prevent Cross-Site Scripting (XSS).
 * @param data The input string to escape.
 * @return The HTML-safe, escaped string.
 */
std::string htmlEscape(const std::string& data) {
    std::string escaped;
    escaped.reserve(data.size()); // Reserve space to avoid multiple reallocations
    for(char c : data) {
        switch(c) {
            case '&':  escaped += "&amp;";       break;
            case '\"': escaped += "&quot;";      break;
            case '\'': escaped += "&#39;";      break;
            case '<':  escaped += "&lt;";        break;
            case '>':  escaped += "&gt;";        break;
            default:   escaped += c;
        }
    }
    return escaped;
}

/**
 * Renders the hello page for a given username.
 * @param username The username to display.
 * @return The full HTML page as a string.
 */
std::string renderHelloPage(const std::string& username) {
    std::string safeUsername = htmlEscape(username);
    std::string content = HELLO_TEMPLATE;
    // Replace the placeholder with the sanitized username
    size_t pos = content.find("{{username}}");
    if (pos != std::string::npos) {
        content.replace(pos, 12, safeUsername);
    }
    return content;
}

/**
 * Simulates a URL router. It maps a URL path to a function.
 * This is the equivalent of a decorator in other languages.
 * @param path The URL path from the request.
 * @return The HTTP response body as a string.
 */
std::string routeRequest(const std::string& path) {
    // Regex to match "/hello/<username>" and capture the username part.
    std::regex re("/hello/(.+)");
    std::smatch match;

    if (std::regex_match(path, match, re)) {
        if (match.size() > 1) {
            std::string username = match[1].str();
            return renderHelloPage(username);
        }
    }
    return "404 Not Found";
}

int main() {
    std::cout << "--- Running 5 Test Cases ---" << std::endl;

    std::vector<std::string> test_paths = {
        // Test Case 1: Simple username
        "/hello/World",
        // Test Case 2: Username with special characters
        "/hello/Alice & Bob",
        // Test Case 3: Numeric username
        "/hello/123-456",
        // Test Case 4: Non-matching URL
        "/about/us",
        // Test Case 5: Malicious XSS attempt (should be escaped)
        "/hello/<script>malware()</script>"
    };

    for (int i = 0; i < test_paths.size(); ++i) {
        std::cout << "Test Case " << i + 1 << " (Input Path: " << test_paths[i] << "):" << std::endl;
        std::string response = routeRequest(test_paths[i]);
        std::cout << response << std::endl;
        std::cout << "--------------------" << std::endl;
    }

    return 0;
}