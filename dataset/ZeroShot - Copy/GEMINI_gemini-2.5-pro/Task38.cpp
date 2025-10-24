#include <iostream>
#include <string>
#include <vector>

/**
 * Securely escapes user input to prevent Cross-Site Scripting (XSS) and
 * wraps it in a basic HTML structure for display.
 *
 * @param userInput The raw string from the user.
 * @return A string containing a complete HTML page with the sanitized input.
 */
std::string secureDisplay(const std::string& userInput) {
    std::string sanitizedInput;
    sanitizedInput.reserve(userInput.length()); 

    for (char c : userInput) {
        switch (c) {
            case '<':
                sanitizedInput.append("&lt;");
                break;
            case '>':
                sanitizedInput.append("&gt;");
                break;
            case '&':
                sanitizedInput.append("&amp;");
                break;
            case '"':
                sanitizedInput.append("&quot;");
                break;
            case '\'':
                sanitizedInput.append("&#39;");
                break;
            default:
                sanitizedInput += c;
                break;
        }
    }
    return "<html><body><h1>User Input:</h1><p>" + sanitizedInput + "</p></body></html>";
}

int main() {
    // 5 Test Cases
    std::vector<std::string> testCases = {
        "Hello, World!", // Normal input
        "<script>alert('XSS');</script>", // Malicious script
        "John & Doe > Company", // Input with special HTML characters
        "This is a \"quote\".", // Input with quotes
        "1 < 2 && 3 > 1" // Another input with special characters
    };

    std::cout << "--- Running C++ Test Cases ---" << std::endl;
    for (size_t i = 0; i < testCases.size(); ++i) {
        std::cout << "\nTest Case " << (i + 1) << ":" << std::endl;
        std::cout << "Original Input: " << testCases[i] << std::endl;
        std::string safeHtml = secureDisplay(testCases[i]);
        std::cout << "Generated Safe HTML: " << safeHtml << std::endl;
    }

    return 0;
}