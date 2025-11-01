
#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cctype>

// Maximum request body size to prevent resource exhaustion
const size_t MAX_REQUEST_BODY_SIZE = 8192;
// Maximum username length
const size_t MAX_USERNAME_LENGTH = 256;

// HTML entity encoding to prevent XSS
std::string sanitize_html(const std::string& input) {
    std::string output;
    output.reserve(input.size() * 2); // Reserve space for potential expansion
    
    for (char c : input) {
        switch (c) {
            case '&':
                output.append("&amp;");
                break;
            case '<':
                output.append("&lt;");
                break;
            case '>':
                output.append("&gt;");
                break;
            case '"':
                output.append("&quot;");
                break;
            case '\\'':
                output.append("&#x27;");
                break;
            case '/':
                output.append("&#x2F;");
                break;
            default:
                output.push_back(c);
                break;
        }
    }
    return output;
}

// Trim whitespace from string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \\t\\n\\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \\t\\n\\r");
    return str.substr(first, (last - first + 1));
}

// Simulated HTTP request structure
struct HttpRequest {
    std::string method;
    std::string path;
    std::string body;
};

// Simulated HTTP response structure
struct HttpResponse {
    int status_code;
    std::string headers;
    std::string body;
};

// Function to handle /hello endpoint
HttpResponse http_hello_page(const HttpRequest& request) {
    HttpResponse response;
    
    // Only accept POST requests
    if (request.method != "POST") {
        response.status_code = 405;
        response.headers = "Content-Type: text/plain; charset=UTF-8\\r\\n";
        response.body = "Method Not Allowed";
        return response;
    }
    
    // Validate request body size
    if (request.body.size() > MAX_REQUEST_BODY_SIZE) {
        response.status_code = 413;
        response.headers = "Content-Type: text/plain; charset=UTF-8\\r\\n";
        response.body = "Request Entity Too Large";
        return response;
    }
    
    // Get username from request body
    std::string username = request.body;
    
    // Validate username: not empty after trimming
    username = trim(username);
    if (username.empty()) {
        response.status_code = 400;
        response.headers = "Content-Type: text/plain; charset=UTF-8\\r\\n";
        response.body = "Invalid Request";
        return response;
    }
    
    // Enforce maximum username length
    if (username.length() > MAX_USERNAME_LENGTH) {
        response.status_code = 400;
        response.headers = "Content-Type: text/plain; charset=UTF-8\\r\\n";
        response.body = "Invalid Request";
        return response;
    }
    
    // Sanitize username to prevent XSS
    std::string username_safe = sanitize_html(username);
    
    // Construct response HTML
    std::ostringstream outputbuf;
    outputbuf << "<!DOCTYPE html>\\n"
              << "<html lang=\\"en\\">\\n"
              << "<head>\\n"
              << "<meta charset=\\"UTF-8\\">\\n"
              << "<meta name=\\"viewport\\" content=\\"width=device-width, initial-scale=1.0\\">\\n"
              << "<title>Hello Page</title>\\n"
              << "</head>\\n"
              << "<body>\\n"
              << "<h1>Hello, " << username_safe << "!</h1>\\n"
              << "</body>\\n"
              << "</html>";
    
    // Set response
    response.status_code = 200;
    response.headers = "Content-Type: text/html; charset=UTF-8\\r\\n"
                      "X-Content-Type-Options: nosniff\\r\\n"
                      "X-Frame-Options: DENY\\r\\n"
                      "Content-Security-Policy: default-src 'self'\\r\\n";
    response.body = outputbuf.str();
    
    return response;
}

// Test function
void test_http_hello_page(const std::string& username) {
    HttpRequest request;
    request.method = "POST";
    request.path = "/hello";
    request.body = username;
    
    HttpResponse response = http_hello_page(request);
    
    std::cout << "Test username: " << username.substr(0, std::min(username.length(), size_t(50))) << std::endl;
    std::cout << "Response code: " << response.status_code << std::endl;
    std::cout << response.body << std::endl;
    std::cout << std::endl;
}

int main() {
    // Test cases
    test_http_hello_page("Alice");
    test_http_hello_page("Bob123");
    test_http_hello_page("<script>alert('xss')</script>");
    test_http_hello_page(std::string(300, 'A')); // Long username
    test_http_hello_page(""); // Empty username
    
    return 0;
}
