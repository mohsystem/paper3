#include <iostream>
#include <string>
#include <sstream>
#include <vector>

/**
 * @brief Sanitizes a string to prevent Cross-Site Scripting (XSS) by escaping HTML special characters.
 * 
 * @param input The raw string to be sanitized.
 * @return std::string The sanitized string with HTML entities.
 */
std::string sanitizeHtml(const std::string& input) {
    std::stringstream ss;
    for (char c : input) {
        switch (c) {
            case '<': ss << "&lt;"; break;
            case '>': ss << "&gt;"; break;
            case '&': ss << "&amp;"; break;
            case '"': ss << "&quot;"; break;
            case '\'': ss << "&#39;"; break;
            default: ss << c; break;
        }
    }
    return ss.str();
}

/**
 * @brief Reads a username from a request stream, constructs a "Hello [username]" message,
 *        and writes it to a response stream. The username is sanitized to prevent XSS.
 * 
 * @param request An input stream (e.g., std::cin, std::ifstream, std::stringstream) 
 *                containing the client request with the username on a single line.
 * @param response An output stream (e.g., std::cout, std::ofstream, std::stringstream)
 *                 to write the response to.
 */
void http_hello_page(std::istream& request, std::ostream& response) {
    // Read the username from the request stream.
    std::string username;
    std::getline(request, username);

    // Sanitize the username to prevent XSS vulnerabilities.
    std::string sanitizedUsername = sanitizeHtml(username);
    
    // Construct and write the response to the response stream.
    response << "Hello " << sanitizedUsername;
}

void run_test_case(int test_num, const std::string& test_input, const std::string& expected_output) {
    std::cout << "Test Case " << test_num << ":" << std::endl;
    std::cout << "Input: \"" << test_input << "\"" << std::endl;

    std::stringstream request_stream(test_input);
    std::stringstream response_stream;

    http_hello_page(request_stream, response_stream);
    std::string actual_output = response_stream.str();

    std::cout << "Expected: \"" << expected_output << "\"" << std::endl;
    std::cout << "Actual  : \"" << actual_output << "\"" << std::endl;
    std::cout << "Result  : " << (expected_output == actual_output ? "PASS" : "FAIL") << std::endl;
    std::cout << "--------------------" << std::endl;
}

int main() {
    // Test cases
    run_test_case(1, "Alice", "Hello Alice");
    run_test_case(2, "<script>alert('xss')</script>", "Hello &lt;script&gt;alert('xss')&lt;/script&gt;");
    run_test_case(3, "Bob & Carol", "Hello Bob &amp; Carol");
    run_test_case(4, "Mr. \"O'Malley\"", "Hello &quot;O&#39;Malley&quot;");
    run_test_case(5, "", "Hello ");
    return 0;
}