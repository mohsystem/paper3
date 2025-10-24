#include <iostream>
#include <string>
#include <sstream>

/**
 * Escapes special HTML characters in a string to prevent XSS.
 *
 * @param input The raw string to be escaped.
 * @return The HTML-escaped string.
 */
std::string escapeHtml(const std::string& input) {
    std::stringstream ss;
    for (char c : input) {
        switch (c) {
            case '&':  ss << "&amp;";       break;
            case '<':  ss << "&lt;";        break;
            case '>':  ss << "&gt;";        break;
            case '"':  ss << "&quot;";      break;
            case '\'': ss << "&#39;";       break; // or &apos;
            default:   ss << c;             break;
        }
    }
    return ss.str();
}

/**
 * Generates a simple HTML page string displaying user-supplied input securely.
 *
 * @param userInput The user-supplied string.
 * @return A string containing the full HTML page.
 */
std::string generateWebPage(const std::string& userInput) {
    std::string escapedInput = escapeHtml(userInput);
    std::stringstream webPage;
    webPage << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<head>\n"
            << "  <title>User Input</title>\n"
            << "</head>\n"
            << "<body>\n"
            << "  <h1>User Comment:</h1>\n"
            << "  <p>" << escapedInput << "</p>\n"
            << "</body>\n"
            << "</html>";
    return webPage.str();
}

int main() {
    std::string testCases[] = {
        "Hello, World!",
        "1 < 5",
        "This is a 'quote' & an \"ampersand\"",
        "<script>alert('XSS Attack!')</script>",
        ""
    };

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "\n--- Test Case " << (i + 1) << " ---" << std::endl;
        std::cout << "Input: " << testCases[i] << std::endl;
        std::string webPage = generateWebPage(testCases[i]);
        std::cout << "Generated HTML:\n" << webPage << std::endl;
    }

    return 0;
}