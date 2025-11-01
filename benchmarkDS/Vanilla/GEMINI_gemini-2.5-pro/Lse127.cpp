#include <iostream>
#include <string>
#include <vector>
#include <sstream> // For string streams

/**
 * @brief Reads a username from the request stream, constructs a "Hello [username]" message,
 *        and writes it to the response stream.
 * 
 * @param request An input stream to read the client's request from.
 * @param response An output stream to write the response to.
 */
void http_hello_page(std::istream& request, std::ostream& response) {
    std::string username;
    if (std::getline(request, username)) {
        response << "Hello " << username << std::endl;
    }
}

int main() {
    // Test cases
    std::vector<std::string> testUsernames = {"Alice", "Bob", "Charlie", "David Smith", "Eve123"};

    std::cout << "--- C++ Test Cases ---" << std::endl;
    for (int i = 0; i < testUsernames.size(); ++i) {
        const std::string& username = testUsernames[i];
        
        // Simulate the request stream
        std::istringstream request_stream(username);

        // Simulate the response stream
        std::ostringstream response_stream;

        std::cout << "Test Case " << (i + 1) << ":" << std::endl;
        std::cout << "Input Username: " << username << std::endl;
        
        // Call the function
        http_hello_page(request_stream, response_stream);
        
        // Get the result and print it
        std::string result = response_stream.str();
        // Trim trailing newline for consistent comparison
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }

        std::cout << "Generated Response: " << result << std::endl;
        std::cout << "Expected Response: Hello " << username << std::endl;
        std::cout << "-------------------------" << std::endl;
    }

    return 0;
}