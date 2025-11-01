#include <iostream>
#include <string>
#include <vector>

/**
 * @brief Sanitizes a string by escaping characters that have special meaning in HTML.
 * This is a crucial security measure to prevent Cross-Site Scripting (XSS) attacks.
 *
 * @param input The string to be sanitized.
 * @return The sanitized HTML-safe string.
 */
std::string escapeHtml(const std::string& input) {
    std::string escaped_str;
    escaped_str.reserve(input.length()); // Pre-allocate memory to improve performance
    for (char c : input) {
        switch (c) {
            case '<':  escaped_str += "&lt;";   break;
            case '>':  escaped_str += "&gt;";   break;
            case '&':  escaped_str += "&amp;";  break;
            case '"':  escaped_str += "&quot;"; break;
            case '\'': escaped_str += "&#39;";  break;
            default:   escaped_str += c;       break;
        }
    }
    return escaped_str;
}

/**
 * @brief Simulates handling an HTTP request for /hello.
 * It takes a username, securely escapes it to prevent XSS,
 * and constructs an HTML response.
 *
 * @param username The username read from the request body.
 * @return A string containing the full HTML page response.
 */
std::string http_hello_page(const std::string& username) {
    // Securely escape the username to prevent XSS vulnerabilities.
    std::string sanitizedUsername = escapeHtml(username);

    // Construct the response body (the HTML page).
    std::string outputbuf = "<html><body><h1>Hello, " + sanitizedUsername + "!</h1></body></html>";

    // In a real server, this would be written to the response stream.
    // Here, we just return it.
    return outputbuf;
}

int main() {
    std::vector<std::string> testCases = {
        "Alice",
        "Bob Smith",
        "",
        "<script>alert('XSS Attack!')</script>",
        "O'Malley & Sons"
    };

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input:  " << testCases[i] << std::endl;
        std::string response = http_hello_page(testCases[i]);
        std::cout << "Output: " << response << std::endl << std::endl;
    }

    return 0;
}