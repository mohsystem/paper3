#include <iostream>
#include <string>
#include <memory>
// This code requires the 'cpp-httplib' single-header library.
// You can get it from: https://github.com/yhirose/cpp-httplib
// To compile, place httplib.h in the same directory and run:
// g++ -o server Lse126.cpp -lpthread -std=c++11
#include "httplib.h"

// Security: Helper function to escape HTML special characters to prevent XSS.
std::string escapeHTML(const std::string& data) {
    std::string buffer;
    buffer.reserve(data.size());
    for(size_t pos = 0; pos != data.size(); ++pos) {
        switch(data[pos]) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&#39;");       break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            default:   buffer.append(&data[pos], 1); break;
        }
    }
    return buffer;
}


/**
 * The core logic of constructing the response page.
 * It's called when a request to /hello is received.
 * @param req The HTTP request object.
 * @param res The HTTP response object.
 */
void http_hello_page(const httplib::Request& req, httplib::Response& res) {
    // The server reads the request body (the username) into the username variable.
    std::string username = req.body;
    
    // Security: Escape the user-provided username.
    std::string escaped_username = escapeHTML(username);

    // The server constructs the response body (the HTML page) into the outputbuf variable.
    std::string outputbuf = "<html><head><title>Hello</title></head>"
                            "<body><h1>Hello, " + escaped_username + "!</h1></body></html>";

    // The server writes the response body to the response stream.
    // The library handles closing the stream and other connection details.
    res.set_content(outputbuf, "text/html");

    // Security: Add basic security headers
    res.set_header("X-Content-Type-Options", "nosniff");
    res.set_header("Content-Security-Policy", "default-src 'self'");
}

int main(void) {
    httplib::Server svr;

    // The function http_hello_page is called when the server receives a POST request for the URL /hello.
    svr.Post("/hello", http_hello_page);
    
    // Security: Set a maximum payload size to prevent DoS attacks.
    svr.set_payload_max_length(1024); // 1 KB limit

    int port = 8080;
    std::cout << "Server started on port " << port << std::endl;
    std::cout << "Run the following commands in another terminal to test:" << std::endl;
    
    // Test cases documentation
    std::cout << "\n--- 5 Test Cases ---" << std::endl;
    std::cout << "1. Normal name:" << std::endl;
    std::cout << "   curl -X POST -d \"World\" http://localhost:8080/hello" << std::endl;
    std::cout << "\n2. Name with spaces (should be URL encoded or quoted):" << std::endl;
    std::cout << "   curl -X POST -d \"Bob Smith\" http://localhost:8080/hello" << std::endl;
    std::cout << "\n3. Empty name:" << std::endl;
    std::cout << "   curl -X POST -d \"\" http://localhost:8080/hello" << std::endl;
    std::cout << "\n4. Malicious XSS attempt (should be safely escaped):" << std::endl;
    std::cout << "   curl -X POST -d \"<script>alert('xss')</script>\" http://localhost:8080/hello" << std::endl;
    std::cout << "\n5. Name with HTML special characters:" << std::endl;
    std::cout << "   curl -X POST -d \"O'Malley & Sons <Company>\" http://localhost:8080/hello" << std::endl;

    // The server is now ready to handle the next request.
    svr.listen("0.0.0.0", port);

    return 0;
}