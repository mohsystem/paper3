
#include <iostream>
#include <string>
#include <regex>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// Maximum URL length to prevent DoS
const size_t MAX_URL_LENGTH = 2048;
const size_t MAX_REQUEST_SIZE = 8192;

// Strict regex pattern that only matches URLs containing example.com
const std::regex ALLOWED_PATTERN("^https://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/.*)?$");

// URL decode function with validation
std::string urlDecode(const std::string& str) {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        if (str[i] == '%') {
            // Check if there are enough characters for hex digits
            if (i + 2 < str.length()) {
                char hex[3] = {str[i+1], str[i+2], 0};
                char* end = nullptr;
                long value = std::strtol(hex, &end, 16);
                
                // Validate hex digits and value range
                if (end == hex + 2 && value >= 0 && value <= 255) {
                    result += static_cast<char>(value);
                    i += 2;
                } else {
                    return ""; // Invalid encoding
                }
            } else {
                return ""; // Incomplete encoding
            }
        } else if (str[i] == '+') {
            result += ' ';
        } else {
            result += str[i];
        }
    }
    
    return result;
}

// Extract target parameter from query string
std::string extractTargetParameter(const std::string& query) {
    if (query.empty() || query.length() > MAX_URL_LENGTH) {
        return "";
    }
    
    size_t pos = query.find("target=");
    if (pos == std::string::npos) {
        return "";
    }
    
    size_t start = pos + 7; // Length of "target="
    size_t end = query.find('&', start);
    
    std::string encoded = (end == std::string::npos) 
        ? query.substr(start) 
        : query.substr(start, end - start);
    
    return urlDecode(encoded);
}

// Validate if URL matches the allowed pattern
bool isValidRedirect(const std::string& url) {
    if (url.empty() || url.length() > MAX_URL_LENGTH) {
        return false;
    }
    
    return std::regex_match(url, ALLOWED_PATTERN);
}

// Validate and sanitize URL
std::string validateAndSanitizeUrl(const std::string& target) {
    const std::string safeDefault = "/";
    
    if (target.empty()) {
        return safeDefault;
    }
    
    // Trim whitespace
    std::string normalized = target;
    normalized.erase(0, normalized.find_first_not_of(" \\t\\n\\r"));
    normalized.erase(normalized.find_last_not_of(" \\t\\n\\r") + 1);
    
    // Length validation
    if (normalized.length() > MAX_URL_LENGTH) {
        return safeDefault;
    }
    
    // Validate against pattern
    if (isValidRedirect(normalized)) {
        return normalized;
    }
    
    return safeDefault;
}

// Handle HTTP request
void handleRequest(int clientSocket) {
    char buffer[MAX_REQUEST_SIZE];
    std::memset(buffer, 0, sizeof(buffer));
    
    // Read request with size limit
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    
    // Ensure null termination
    buffer[bytesRead] = '\\0';
    
    std::string request(buffer);
    
    // Extract GET request line
    size_t firstLine = request.find("\\r\\n");
    if (firstLine == std::string::npos) {
        close(clientSocket);
        return;
    }
    
    std::string requestLine = request.substr(0, firstLine);
    
    // Parse request method and path
    if (requestLine.find("GET /redirect") != 0) {
        const char* notFound = "HTTP/1.1 404 Not Found\\r\\n\\r\\n";
        send(clientSocket, notFound, std::strlen(notFound), 0);
        close(clientSocket);
        return;
    }
    
    // Extract query string
    size_t queryStart = requestLine.find('?');
    std::string target;
    
    if (queryStart != std::string::npos) {
        size_t queryEnd = requestLine.find(' ', queryStart);
        if (queryEnd != std::string::npos) {
            std::string query = requestLine.substr(queryStart + 1, queryEnd - queryStart - 1);
            target = extractTargetParameter(query);
        }
    }
    
    // Validate and get redirect URL
    std::string redirectUrl = validateAndSanitizeUrl(target);
    
    // Send redirect response with security headers
    std::string response = "HTTP/1.1 302 Found\\r\\n"
                          "Location: " + redirectUrl + "\\r\\n"
                          "X-Content-Type-Options: nosniff\\r\\n"
                          "X-Frame-Options: DENY\\r\\n"
                          "Content-Length: 0\\r\\n"
                          "\\r\\n";
    
    send(clientSocket, response.c_str(), response.length(), 0);
    close(clientSocket);
}

int main() {
    // Test cases
    std::cout << "Test Cases:" << std::endl;
    std::cout << "1. Valid: " << isValidRedirect("https://example.com") << std::endl;
    std::cout << "2. Valid: " << isValidRedirect("https://www.example.com/path") << std::endl;
    std::cout << "3. Valid: " << isValidRedirect("https://subdomain.example.com/page") << std::endl;
    std::cout << "4. Invalid: " << isValidRedirect("https://evil.com") << std::endl;
    std::cout << "5. Invalid: " << isValidRedirect("https://evil.com@example.com") << std::endl;
    
    // Create socket
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }
    
    // Set socket options to reuse address
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    // Bind to port 8000
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8000);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind socket" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    // Listen for connections
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Failed to listen" << std::endl;
        close(serverSocket);
        return 1;
    }
    
    std::cout << "Server started on port 8000" << std::endl;
    
    // Accept and handle connections
    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientLen);
        
        if (clientSocket >= 0) {
            handleRequest(clientSocket);
        }
    }
    
    close(serverSocket);
    return 0;
}
