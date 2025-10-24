#include <iostream>
#include <string>
#include <vector>
#include <sstream>

/**
 * Sanitizes a string by escaping HTML special characters.
 * @param input The raw string.
 * @return The sanitized string with HTML entities.
 */
std::string escapeHtml(const std::string& input) {
    std::stringstream ss;
    for (char c : input) {
        switch (c) {
            case '&':  ss << "&amp;";       break;
            case '<':  ss << "&lt;";        break;
            case '>':  ss << "&gt;";        break;
            case '"':  ss << "&quot;";      break;
            case '\'': ss << "&#39;";       break;
            default:   ss << c;
        }
    }
    return ss.str();
}

/**
 * Sanitizes user input and embeds it into a simple HTML page structure.
 * @param userInput The raw string provided by the user.
 * @return A string containing a full HTML document with the sanitized user input.
 */
std::string generateSafeHtml(const std::string& userInput) {
    std::string sanitizedInput = escapeHtml(userInput);
    std::stringstream html;
    html << "<html><body><p>User input: " << sanitizedInput << "</p></body></html>";
    return html.str();
}

int main() {
    std::vector<std::string> testCases = {
        "Hello, World!",
        "1 < 5 is true",
        "He said, \"It's a & b > c\"",
        "<script>alert('XSS attack!');</script>",
        ""
    };

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (const auto& testCase : testCases) {
        std::cout << "Input: " << testCase << std::endl;
        std::string safeHtml = generateSafeHtml(testCase);
        std::cout << "Output: " << safeHtml << std::endl;
        std::cout << std::endl;
    }
    return 0;
}