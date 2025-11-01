#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <sstream>

const int MAX_USERNAME_LENGTH = 50;
const int MAX_REQUEST_LINES = 10;
// Rule #1: Use a strict pattern for a safe username.
const std::regex USERNAME_PATTERN("^[a-zA-Z0-9_-]+$");

/**
 * @brief Reads a request stream to find a username, validates it, and writes a
 * "hello [username]" HTTP response to the response stream.
 *
 * @param request An std::istream containing the client request.
 * @param response An std::ostream to write the HTTP response to.
 */
void http_hello_page(std::istream& request, std::ostream& response) {
    std::string username;
    std::string line;
    int linesRead = 0;

    // Rule #2, #3: Read line by line with a limit to avoid reading unbounded data.
    while (linesRead < MAX_REQUEST_LINES && std::getline(request, line)) {
        // Remove potential \r from windows-style line endings
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (line.rfind("username=", 0) == 0) { // Check if the line starts with "username="
            username = line.substr(std::string("username=").length());
            break;
        }
        linesRead++;
    }

    // Rule #1: Validate the extracted username for length and content.
    bool isValid = !username.empty() &&
                   username.length() <= MAX_USERNAME_LENGTH &&
                   std::regex_match(username, USERNAME_PATTERN);

    if (isValid) {
        std::string body = "hello " + username;
        response << "HTTP/1.1 200 OK\r\n"
                 << "Content-Type: text/plain; charset=utf-8\r\n"
                 << "Content-Length: " << body.length() << "\r\n"
                 << "\r\n"
                 << body;
    } else {
        std::string body = "Bad Request: Invalid or missing username.";
        response << "HTTP/1.1 400 Bad Request\r\n"
                 << "Content-Type: text/plain; charset=utf-8\r\n"
                 << "Content-Length: " << body.length() << "\r\n"
                 << "\r\n"
                 << body;
    }
    response.flush();
}

void test(const std::string& testName, const std::string& requestPayload) {
    std::cout << "--- Test Case: " << testName << " ---" << std::endl;
    std::stringstream requestStream(requestPayload);
    std::stringstream responseStream;

    http_hello_page(requestStream, responseStream);

    std::cout << "Request:\n" << requestPayload;
    if (requestPayload.empty() || requestPayload.back() != '\n') {
        std::cout << std::endl;
    }
    std::cout << "Response:\n" << responseStream.str() << std::endl << std::endl;
}

int main() {
    test("Valid", "username=Alice\n");
    test("Long Username", "username=" + std::string(MAX_USERNAME_LENGTH + 1, 'a') + "\n");
    test("Invalid Characters", "username=<script>alert('xss')</script>\n");
    test("No Username", "someotherheader=value\n");
    test("Empty Username", "username=\n");
    return 0;
}