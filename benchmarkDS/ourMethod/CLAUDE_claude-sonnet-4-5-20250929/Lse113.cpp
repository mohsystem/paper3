
#include <iostream>
#include <string>
#include <cstring>
#include <regex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

// Security: Using socket-based connectivity check instead of system()
// This prevents command injection vulnerabilities (CWE-78)
std::string ping(const std::string& url) {
    // Input validation: check for empty string
    if (url.empty()) {
        return "Error: Invalid URL provided";
    }
    
    // Sanitize and validate input
    std::string sanitized_url = url;
    
    // Remove leading/trailing whitespace
    size_t start = sanitized_url.find_first_not_of(" \\t\\n\\r");
    size_t end = sanitized_url.find_last_not_of(" \\t\\n\\r");
    if (start == std::string::npos) {
        return "Error: Invalid URL format";
    }
    sanitized_url = sanitized_url.substr(start, end - start + 1);
    
    // Validate URL format - only allow safe characters
    // Reject characters that could be used for command injection
    std::regex valid_pattern("^[a-zA-Z0-9.:\\\\-]+$");
    if (!std::regex_match(sanitized_url, valid_pattern)) {
        return "Error: Invalid URL format";
    }
    
    // Limit URL length to prevent resource exhaustion
    if (sanitized_url.length() > 253) {
        return "Error: URL too long";
    }
    
    // Resolve hostname to IP address
    struct addrinfo hints, *result = nullptr;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(sanitized_url.c_str(), "80", &hints, &result);
    if (status != 0) {
        // Generic error message to avoid leaking details
        return "Error: Unable to resolve host";
    }
    
    // Try to connect using socket - safer than system ping
    int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(result);
        return "Error: Network error occurred";
    }
    
    // Set socket to non-blocking mode for timeout control
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    }
    
    // Attempt connection
    int connect_result = connect(sockfd, result->ai_addr, result->ai_addrlen);
    
    bool is_alive = false;
    if (connect_result < 0) {
        if (errno == EINPROGRESS) {
            // Connection in progress, use select with timeout
            fd_set fdset;
            struct timeval tv;
            FD_ZERO(&fdset);
            FD_SET(sockfd, &fdset);
            tv.tv_sec = 5;  // 5 second timeout
            tv.tv_usec = 0;
            
            if (select(sockfd + 1, nullptr, &fdset, nullptr, &tv) > 0) {
                int so_error = 0;
                socklen_t len = sizeof(so_error);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
                is_alive = (so_error == 0);
            }
        }
    } else {
        is_alive = true;
    }
    
    close(sockfd);
    freeaddrinfo(result);
    
    if (is_alive) {
        return sanitized_url + " is alive";
    } else {
        return sanitized_url + " is not alive";
    }
}

int main() {
    // Test cases
    std::cout << "Test Case 1: Valid domain" << std::endl;
    std::cout << ping("google.com") << std::endl;
    
    std::cout << "\\nTest Case 2: Valid IP" << std::endl;
    std::cout << ping("8.8.8.8") << std::endl;
    
    std::cout << "\\nTest Case 3: Invalid domain" << std::endl;
    std::cout << ping("thisdoesnotexist999.invalid") << std::endl;
    
    std::cout << "\\nTest Case 4: Empty string" << std::endl;
    std::cout << ping("") << std::endl;
    
    std::cout << "\\nTest Case 5: Malicious input attempt" << std::endl;
    std::cout << ping("google.com && cat /etc/passwd") << std::endl;
    
    return 0;
}
