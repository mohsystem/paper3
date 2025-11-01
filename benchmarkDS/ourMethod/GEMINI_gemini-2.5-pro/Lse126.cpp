#include <iostream>
#include <string>
#include <sstream>
#include <vector>

const size_t MAX_USERNAME_LENGTH = 128;

/**
 * @brief Sanitizes a string for HTML context to prevent XSS.
 * @param input The raw string.
 * @return A new string with HTML special characters escaped.
 */
std::string html_escape(const std::string& input) {
    std::stringstream escaped;
    for (char c : input) {
        switch (c) {
            case '&':  escaped << "&amp;"; break;
            case '\"': escaped << "&quot;"; break;
            case '\'': escaped << "&#39;"; break;
            case '<':  escaped << "&lt;"; break;
            case '>':  escaped << "&gt;"; break;
            default:   escaped << c; break;
        }
    }
    return escaped.str();
}

/**
 * @brief Handles a /hello request.
 * Reads a username from the request stream and writes an HTML greeting
 * to the response stream.
 * @param request_stream The input stream containing the request body.
 * @param response_stream The output stream to write the response to.
 */
void http_hello_page(std::istream& request_stream, std::ostream& response_stream) {
    // The server reads the request body (the username) into the username variable.
    // Read up to a safe limit to avoid consuming too much memory.
    std::vector<char> buffer(MAX_USERNAME_LENGTH);
    request_stream.read(buffer.data(), buffer.size());
    std::streamsize bytes_read = request_stream.gcount();
    
    std::string username(buffer.data(), bytes_read);

    if (username.empty()) {
        username = "Guest";
    }
    
    // Sanitize input
    std::string sanitized_username = html_escape(username);

    // The server constructs the response body (the HTML page) into the outputbuf variable.
    std::stringstream outputbuf;
    outputbuf << "<html><body><h1>Hello, " << sanitized_username << "!</h1></body></html>";
    
    // The server writes the response body to the response stream.
    response_stream << outputbuf.rdbuf();

    // The server closes the response stream. In C++, this is typically handled by
    // the object's destructor (RAII), so the caller who owns the stream is
    // responsible for its lifecycle.
}

int main() {
    std::string test_cases[] = {
        "Alice",
        "Bob<script>alert(1)</script>",
        "", // Empty username
        "Charles & David",
        "Eve's \"special\" username that is quite long to test the boundaries and see how it behaves with more characters"
    };
    
    int test_num = 1;
    for (const auto& test_case : test_cases) {
        std::cout << "--- Test Case " << test_num++ << " ---" << std::endl;
        std::cout << "Input Username: " << test_case << std::endl;

        std::stringstream request(test_case);
        std::stringstream response;
        
        http_hello_page(request, response);
        
        std::cout << "Output HTML: " << response.str() << std::endl;
        std::cout << std::endl;
    }

    return 0;
}