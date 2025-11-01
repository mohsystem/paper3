#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <limits>

const size_t MAX_USERNAME_LEN = 256;

/**
 * @brief Escapes characters in a string that are significant in HTML.
 * @param s The string to escape.
 * @return A new string with HTML-sensitive characters escaped.
 */
std::string escapeHtml(const std::string& s) {
    std::string escaped;
    escaped.reserve(s.length());
    for (char c : s) {
        switch (c) {
            case '&':  escaped += "&amp;"; break;
            case '\"': escaped += "&quot;"; break;
            case '\'': escaped += "&#x27;"; break;
            case '<':  escaped += "&lt;"; break;
            case '>':  escaped += "&gt;"; break;
            default:   escaped += c; break;
        }
    }
    return escaped;
}

/**
 * @brief Reads a username, constructs a "Hello" response, and writes it.
 * This function reads the username in a bounded way to prevent DoS attacks
 * and sanitizes it to prevent XSS.
 * @param request The input stream containing the client request.
 * @param response The output stream to send the response to.
 */
void http_hello_page(std::istream& request, std::ostream& response) {
    std::vector<char> buffer(MAX_USERNAME_LEN + 2); // For content, potential newline, and null terminator
    std::string username;

    // Read with a boundary to prevent buffer overflow and DoS.
    request.getline(buffer.data(), buffer.size());
    
    if (request.fail() && !request.eof()) {
        // The line was too long, so failbit is set.
        request.clear(); // Clear the fail state.
        // Consume the rest of the invalid line from the stream.
        request.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        username = "Guest"; // Reject the long input.
    } else {
        username = buffer.data();
        // Trim whitespace
        size_t first = username.find_first_not_of(" \t\n\r");
        if (std::string::npos == first) {
            username = "Guest";
        } else {
            size_t last = username.find_last_not_of(" \t\n\r");
            username = username.substr(first, (last - first + 1));
        }
    }
    
    std::string sanitized_username = escapeHtml(username);
    response << "Hello " << sanitized_username;
    response.flush();
}

void run_test_case(const std::string& name, const std::string& input) {
    std::cout << "--- " << name << " ---" << std::endl;
    std::string printable_input = input;
    if (printable_input.length() > 50) {
        printable_input = printable_input.substr(0, 50) + "...";
    }
    std::cout << "Input: \"" << printable_input << "\"" << std::endl;

    std::stringstream request_stream(input);
    std::stringstream response_stream;

    http_hello_page(request_stream, response_stream);

    std::cout << "Output: \"" << response_stream.str() << "\"" << std::endl;
    std::cout << std::endl;
}

int main() {
    run_test_case("Test Case 1: Normal", "Alice");
    run_test_case("Test Case 2: With Newline", "Bob\nnextline");
    run_test_case("Test Case 3: XSS Attempt", "<script>alert('xss')</script>");
    run_test_case("Test Case 4: Whitespace", "    ");
    std::string long_name(MAX_USERNAME_LEN + 50, 'A');
    run_test_case("Test Case 5: Too Long", long_name);
    return 0;
}