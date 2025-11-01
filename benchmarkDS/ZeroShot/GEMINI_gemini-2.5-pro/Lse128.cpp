#include <iostream>
#include <string>
#include <sstream>
#include <vector>

/**
 * @brief Escapes characters in a string to be safe for inclusion in HTML.
 * This prevents Cross-Site Scripting (XSS) vulnerabilities.
 * 
 * @param input The string to escape.
 * @return The escaped string.
 */
std::string html_escape(const std::string& input) {
    std::string output;
    output.reserve(input.size());
    for(char c : input) {
        switch(c) {
            case '&':  output.append("&amp;");       break;
            case '\"': output.append("&quot;");      break;
            case '\'': output.append("&#39;");       break;
            case '<':  output.append("&lt;");        break;
            case '>':  output.append("&gt;");        break;
            default:   output.push_back(c);          break;
        }
    }
    return output;
}

/**
 * @brief Reads a username from a request stream, constructs a secure "hello [username]"
 * HTML response, and writes it to the response stream.
 * 
 * @param request An input stream for the request, assumed to contain the username.
 * @param response An output stream for the response.
 */
void http_hello_page(std::istream& request, std::ostream& response) {
    // In a real application, you would parse a full HTTP request.
    // For this example, we assume the request stream contains only the username on a single line.
    std::string username;
    std::getline(request, username);
    
    // Securely escape the user-provided username to prevent XSS attacks.
    std::string escaped_username = html_escape(username);
    
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/html; charset=UTF-8\r\n\r\n"
             << "<html><body>hello " << escaped_username << "</body></html>";
    response.flush();
}

int main() {
    std::vector<std::string> test_inputs = {
        "Alice",                                  // Test case 1: Normal input
        "Bob Smith",                              // Test case 2: Input with spaces
        "",                                       // Test case 3: Empty input
        "<script>alert('XSS')</script>",          // Test case 4: XSS attack attempt
        "John&Doe\" onload=\"alert('hacked')\""   // Test case 5: XSS attack with quotes and ampersand
    };

    int test_num = 1;
    for (const auto& input : test_inputs) {
        std::cout << "--- Test Case " << test_num++ << " ---" << std::endl;
        std::cout << "Input: " << input << std::endl;

        // Simulate file streams using stringstream
        std::stringstream request_stream(input);
        std::stringstream response_stream;

        http_hello_page(request_stream, response_stream);

        std::cout << "Output:" << std::endl;
        std::cout << response_stream.str() << std::endl << std::endl;
    }

    return 0;
}