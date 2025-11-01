#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <utility>

/**
 * @brief Sanitizes a string for HTML output to prevent XSS.
 *
 * Replaces special HTML characters with their corresponding entities.
 * @param input The raw string to be sanitized.
 * @return A new string that is safe to embed in HTML.
 */
std::string escapeHtml(const std::string& input) {
    std::stringstream escaped;
    for (char c : input) {
        switch (c) {
            case '<':  escaped << "&lt;";   break;
            case '>':  escaped << "&gt;";   break;
            case '&':  escaped << "&amp;";  break;
            case '"':  escaped << "&quot;"; break;
            case '\'': escaped << "&#39;"; break;
            default:   escaped << c;      break;
        }
    }
    return escaped.str();
}

/**
 * @brief Reads a username from a request stream, sanitizes it, and writes a
 * "hello [username]" message to the response stream.
 *
 * @param request An input stream (e.g., std::cin, std::ifstream, std::stringstream).
 * @param response An output stream (e.g., std::cout, std::ofstream, std::stringstream).
 */
void http_hello_page(std::istream& request, std::ostream& response) {
    std::string username;
    
    // Safely read a line from the input stream. Handles varying lengths without buffer overflows.
    if (std::getline(request, username)) {
        // Sanitize the username to prevent XSS
        std::string sanitizedUsername = escapeHtml(username);
        
        // Construct and write the response
        response << "hello " << sanitizedUsername;
    } else {
        // Handle case where request stream is empty or fails
        response << "hello ";
    }
}

int main() {
    // Test cases: {input_username, expected_output}
    std::vector<std::pair<std::string, std::string>> test_cases = {
        {"Alice", "hello Alice"},
        {"Bob Smith", "hello Bob Smith"},
        {"", "hello "},
        {"<script>alert('XSS')</script>", "hello &lt;script&gt;alert(&#39;XSS&#39;)&lt;/script&gt;"},
        {"User with \"&'<> special chars", "hello User with &quot;&amp;&#39;&lt;&gt; special chars"}
    };

    std::cout << "--- Running C++ Tests ---" << std::endl;
    int test_num = 1;
    for (const auto& tc : test_cases) {
        const std::string& username = tc.first;
        const std::string& expected = tc.second;

        // Simulate request and response streams using std::stringstream
        std::stringstream request_stream(username);
        std::stringstream response_stream;

        // Call the function
        http_hello_page(request_stream, response_stream);

        // Get the actual output
        std::string actual = response_stream.str();
        
        // Compare and print results
        std::cout << "Test Case " << test_num++ << ":" << std::endl;
        std::cout << "  Input:    \"" << username << "\"" << std::endl;
        std::cout << "  Expected: \"" << expected << "\"" << std::endl;
        std::cout << "  Actual:   \"" << actual << "\"" << std::endl;
        std::cout << "  Result:   " << (expected == actual ? "PASSED" : "FAILED") << std::endl;
        std::cout << std::endl;
    }

    return 0;
}