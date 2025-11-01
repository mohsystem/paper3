
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>

#define MAX_URL_LENGTH 253
#define TIMEOUT_SECONDS 5

// Security: Using socket-based connectivity check instead of system()
// This prevents command injection vulnerabilities (CWE-78)
char* ping(const char* url) {
    // Allocate memory for result string
    static char result[512];
    memset(result, 0, sizeof(result));
    
    // Input validation: check for NULL or empty string
    if (url == NULL || url[0] == '\\0') {
        snprintf(result, sizeof(result), "Error: Invalid URL provided");
        return result;
    }
    
    // Validate URL length to prevent buffer overflow
    size_t url_len = strlen(url);
    if (url_len > MAX_URL_LENGTH) {
        snprintf(result, sizeof(result), "Error: URL too long");
        return result;
    }
    
    // Allocate and sanitize URL (remove whitespace)
    char sanitized_url[MAX_URL_LENGTH + 1];
    memset(sanitized_url, 0, sizeof(sanitized_url));
    
    size_t j = 0;
    for (size_t i = 0; i < url_len && j < MAX_URL_LENGTH; i++) {
        if (!isspace((unsigned char)url[i])) {
            sanitized_url[j++] = url[i];
        }
    }
    sanitized_url[j] = '\\0';
    
    if (sanitized_url[0] == '\\0') {
        snprintf(result, sizeof(result), "Error: Invalid URL format");
        return result;
    }
    
    // Validate characters - only allow alphanumeric, dots, hyphens, and colons
    // This prevents command injection attempts
    for (size_t i = 0; sanitized_url[i] != '\\0'; i++) {
        char c = sanitized_url[i];
        if (!isalnum((unsigned char)c) && c != '.' && c != '-' && c != ':') {
            snprintf(result, sizeof(result), "Error: Invalid URL format");
            return result;
        }
    }
    
    // Resolve hostname using getaddrinfo (safer than gethostbyname)
    struct addrinfo hints, *res = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(sanitized_url, "80", &hints, &res);
    if (status != 0) {
        // Generic error message to avoid leaking internal details
        snprintf(result, sizeof(result), "Error: Unable to resolve host");
        return result;
    }
    
    // Create socket
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        snprintf(result, sizeof(result), "Error: Network error occurred");
        return result;
    }
    
    // Set socket to non-blocking for timeout control
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags >= 0) {
        fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    }
    
    // Attempt connection
    int connect_result = connect(sockfd, res->ai_addr, res->ai_addrlen);
    
    int is_alive = 0;
    if (connect_result < 0) {
        if (errno == EINPROGRESS) {
            // Connection in progress, use select with timeout
            fd_set fdset;
            struct timeval tv;
            FD_ZERO(&fdset);
            FD_SET(sockfd, &fdset);
            tv.tv_sec = TIMEOUT_SECONDS;
            tv.tv_usec = 0;
            
            if (select(sockfd + 1, NULL, &fdset, NULL, &tv) > 0) {
                int so_error = 0;
                socklen_t len = sizeof(so_error);
                getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &so_error, &len);
                is_alive = (so_error == 0);
            }
        }
    } else {
        is_alive = 1;
    }
    
    close(sockfd);
    freeaddrinfo(res);
    
    if (is_alive) {
        snprintf(result, sizeof(result), "%s is alive", sanitized_url);
    } else {
        snprintf(result, sizeof(result), "%s is not alive", sanitized_url);
    }
    
    return result;
}

int main(void) {
    // Test cases
    printf("Test Case 1: Valid domain\\n");
    printf("%s\\n", ping("google.com"));
    
    printf("\\nTest Case 2: Valid IP\\n");
    printf("%s\\n", ping("8.8.8.8"));
    
    printf("\\nTest Case 3: Invalid domain\\n");
    printf("%s\\n", ping("thisdoesnotexist999.invalid"));
    
    printf("\\nTest Case 4: Empty string\\n");
    printf("%s\\n", ping(""));
    
    printf("\\nTest Case 5: Malicious input attempt\\n");
    printf("%s\\n", ping("google.com && cat /etc/passwd"));
    
    return 0;
}
