#include <iostream>
#include <string>
#include <vector>

/**
 * Escapes special characters in a string for use in HTML.
 * This is crucial to prevent Cross-Site Scripting (XSS) attacks.
 *
 * @param input The raw string to be escaped.
 * @return The HTML-escaped string.
 */
std::string htmlEscape(const std::string& input) {
    std::string escaped_str;
    escaped_str.reserve(input.length());
    for (char c : input) {
        switch (c) {
            case '&':  escaped_str.append("&amp;");       break;
            case '\"': escaped_str.append("&quot;");      break;
            case '\'': escaped_str.append("&#39;");       break;
            case '<':  escaped_str.append("&lt;");        break;
            case '>':  escaped_str.append("&gt;");        break;
            default:   escaped_str.push_back(c);         break;
        }
    }
    return escaped_str;
}

/**
 * Generates an HTML page displaying the user-provided (and now escaped) input.
 * Note: C++ has no standard library for a web server. This function
 * generates the HTML content that a web server would serve.
 *
 * @param userInput The user-provided string.
 * @return A full HTML page as a std::string.
 */
std::string generateWebPage(const std::string& userInput) {
    std::string escapedInput = htmlEscape(userInput);
    std::string html_content =
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <title>User Input Display</title>\n"
        "    <meta charset=\"UTF-8\">\n"
        "</head>\n"
        "<body>\n"
        "    <h1>User-Supplied Input:</h1>\n"
        "    <p style=\"border: 1px solid black; padding: 10px; background-color: #f0f0f0;\">\n"
        "        " + escapedInput + "\n"
        "    </p>\n"
        "</body>\n"
        "</html>";
    return html_content;
}

void runTestCase(const std::string& testName, const std::string& input) {
    std::cout << "--- " << testName << " ---\n";
    std::string page = generateWebPage(input);
    std::cout << page << std::endl << std::endl;
}

int main() {
    // C++ has no standard library for networking. The core security task is
    // to correctly escape user input before embedding it in HTML. The test
    // cases below demonstrate this by generating the HTML as a string and
    // printing it to the console.

    // Test Case 1: Simple text
    runTestCase("Test Case 1 (Simple Text)", "Hello, world!");

    // Test Case 2: Text with HTML characters
    runTestCase("Test Case 2 (HTML Chars)", "<p>This is a paragraph.</p>");

    // Test Case 3: XSS attack attempt
    runTestCase("Test Case 3 (XSS Attempt)", "<script>alert('XSS');</script>");

    // Test Case 4: Text with quotes and ampersand
    runTestCase("Test Case 4 (Quotes & Ampersand)", "AT&T's new motto is \"Rethink Possible\".");

    // Test Case 5: Empty input
    runTestCase("Test Case 5 (Empty Input)", "");

    return 0;
}