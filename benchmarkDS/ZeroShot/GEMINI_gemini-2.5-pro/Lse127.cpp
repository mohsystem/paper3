#include <iostream>
#include <string>
#include <sstream>
#include <vector>

// Define a reasonable maximum length for a username
const size_t MAX_USERNAME_LENGTH = 256;

/**
 * Performs HTML entity encoding to prevent Cross-Site Scripting (XSS).
 *
 * @param data The raw string to encode.
 * @return The HTML-encoded string.
 */
std::string encodeHTML(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for (char c : data) {
        switch (c) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&#x27;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            case '/':  buffer.append("&#x2F;");      break;
            default:   buffer.push_back(c);         break;
        }
    }
    return buffer;
}

/**
 * Reads a username from a request stream, validates it, and writes a secure
 * "Hello [username]" greeting to a response stream.
 *
 * @param request  An input stream representing the client's request.
 * @param response An output stream representing the server's response.
 */
void http_hello_page(std::istream& request, std::ostream& response) {
    std::string username;
    
    // Read the username. std::getline is safe against buffer overflows.
    if (!std::getline(request, username)) {
        // Handle stream error or EOF
        response << "Hello stranger! (Error reading name)";
        return;
    }
    
    // 1. Security: Validate input length.
    if (username.length() > MAX_USERNAME_LENGTH) {
        response << "Hello! (Username is too long)";
        return;
    }

    // 2. Security: Sanitize output by HTML-encoding the username.
    std::string sanitizedUsername = encodeHTML(username);

    // Construct and write the response
    response << "Hello " << sanitizedUsername;
}

void run_test_case(int caseNum, const std::string& input) {
    std::cout << "--- Test Case " << caseNum << " ---" << std::endl;
    std::cout << "Input: " << input << std::endl;

    // Use stringstream to simulate file streams
    std::stringstream request_stream(input);
    std::stringstream response_stream;

    http_hello_page(request_stream, response_stream);

    std::cout << "Output: " << response_stream.str() << std::endl;
    std::cout << std::endl;
}

int main() {
    // Test cases
    std::vector<std::string> test_inputs = {
        // Test Case 1: Normal valid username
        "Alice",
        // Test Case 2: XSS attack attempt
        "<script>alert('xss')</script>",
        // Test Case 3: Input length violation
        std::string(300, 'A'),
        // Test Case 4: Empty username
        "",
        // Test Case 5: Username with special characters that need encoding
        "Bob&Eve"
    };

    for (size_t i = 0; i < test_inputs.size(); ++i) {
        run_test_case(i + 1, test_inputs[i]);
    }

    return 0;
}