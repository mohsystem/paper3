
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cctype>
#include <cstring>

// Maximum allowed lengths to prevent resource exhaustion
constexpr size_t MAX_USERNAME_LENGTH = 100;
constexpr size_t MAX_REQUEST_SIZE = 1024;

/**
 * Sanitizes username to prevent injection attacks
 * Allows only alphanumeric characters, spaces, hyphens, and underscores
 * @param username The username to sanitize
 * @return Sanitized username
 */
std::string sanitizeUsername(const std::string& username) {
    if (username.empty()) {
        return "Guest";
    }
    
    std::string sanitized;
    sanitized.reserve(username.length());
    
    // Allow only safe characters
    for (char c : username) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == ' ' || c == '-' || c == '_') {
            sanitized += c;
        }
    }
    
    // Trim whitespace from both ends
    size_t start = sanitized.find_first_not_of(" \\t\\r\\n");
    size_t end = sanitized.find_last_not_of(" \\t\\r\\n");
    
    if (start == std::string::npos) {
        return "Guest";
    }
    
    sanitized = sanitized.substr(start, end - start + 1);
    return sanitized.empty() ? "Guest" : sanitized;
}

/**
 * Extracts username from request string
 * @param request The request string
 * @return Extracted username or "Guest" if not found
 */
std::string extractUsername(const std::string& request) {
    if (request.empty()) {
        return "Guest";
    }
    
    // Look for username parameter
    size_t pos = request.find("username=");
    if (pos == std::string::npos) {
        return "Guest";
    }
    
    // Get substring after "username="
    pos += 9; // Length of "username="
    if (pos >= request.length()) {
        return "Guest";
    }
    
    std::string username = request.substr(pos);
    
    // Find end of username (at delimiter or end of string)
    size_t end = username.find_first_of("&\\n\\r");
    if (end != std::string::npos) {
        username = username.substr(0, end);
    }
    
    return username;
}

/**
 * Processes HTTP request and sends greeting response
 * @param request Input stream containing the client request
 * @param response Output stream for sending response to client
 */
void httpHelloPage(std::istream& request, std::ostream& response) {
    if (!request.good() || !response.good()) {
        throw std::invalid_argument("Request and response streams must be valid");
    }
    
    // Read request with size limit to prevent memory exhaustion
    char buffer[MAX_REQUEST_SIZE + 1];
    std::memset(buffer, 0, sizeof(buffer));
    
    request.read(buffer, MAX_REQUEST_SIZE);
    std::streamsize bytesRead = request.gcount();
    
    // Ensure null termination
    buffer[bytesRead] = '\\0';
    
    std::string requestStr(buffer, bytesRead);
    
    // Extract username - validate and sanitize input
    std::string username = extractUsername(requestStr);
    
    // Validate username length
    if (username.length() > MAX_USERNAME_LENGTH) {
        username = username.substr(0, MAX_USERNAME_LENGTH);
    }
    
    // Sanitize username to prevent injection attacks
    username = sanitizeUsername(username);
    
    // Construct response with sanitized data
    std::string responseMessage = "Hello " + username;
    
    // Write response
    response.write(responseMessage.c_str(), responseMessage.length());
    response.flush();
}

int main() {
    // Test case 1: Normal username
    try {
        std::istringstream req1("username=Alice");
        std::ostringstream resp1;
        httpHelloPage(req1, resp1);
        std::cout << "Test 1: " << resp1.str() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 1 failed: " << e.what() << std::endl;
    }
    
    // Test case 2: Username with special characters (should be sanitized)
    try {
        std::istringstream req2("username=Bob<script>");
        std::ostringstream resp2;
        httpHelloPage(req2, resp2);
        std::cout << "Test 2: " << resp2.str() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 2 failed: " << e.what() << std::endl;
    }
    
    // Test case 3: No username provided
    try {
        std::istringstream req3("other=data");
        std::ostringstream resp3;
        httpHelloPage(req3, resp3);
        std::cout << "Test 3: " << resp3.str() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 3 failed: " << e.what() << std::endl;
    }
    
    // Test case 4: Very long username (should be truncated)
    try {
        std::string longName = "username=" + std::string(200, 'A');
        std::istringstream req4(longName);
        std::ostringstream resp4;
        httpHelloPage(req4, resp4);
        std::cout << "Test 4: " << resp4.str() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 4 failed: " << e.what() << std::endl;
    }
    
    // Test case 5: Username with valid special characters
    try {
        std::istringstream req5("username=John_Doe-123");
        std::ostringstream resp5;
        httpHelloPage(req5, resp5);
        std::cout << "Test 5: " << resp5.str() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Test 5 failed: " << e.what() << std::endl;
    }
    
    return 0;
}
