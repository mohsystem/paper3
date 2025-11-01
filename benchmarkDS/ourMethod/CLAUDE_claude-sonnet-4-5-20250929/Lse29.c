
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <regex.h>
#include <ctype.h>

#define MAX_URL_LENGTH 2048
#define MAX_REQUEST_SIZE 8192
#define PATTERN "^https://([a-zA-Z0-9-]+\\\\.)*example\\\\.com(/.*)?$"

/* URL decode with validation */
int urlDecode(const char* src, char* dest, size_t destSize) {
    size_t i = 0, j = 0;
    
    if (!src || !dest || destSize == 0) {
        return -1;
    }
    
    while (src[i] != '\\0' && j < destSize - 1) {
        if (src[i] == '%') {
            /* Check for valid hex digits */
            if (src[i+1] != '\\0' && src[i+2] != '\\0' &&
                isxdigit((unsigned char)src[i+1]) && 
                isxdigit((unsigned char)src[i+2])) {
                
                char hex[3] = {src[i+1], src[i+2], '\\0'};
                char* end = NULL;
                long value = strtol(hex, &end, 16);
                
                if (end == hex + 2 && value >= 0 && value <= 255) {
                    dest[j++] = (char)value;
                    i += 3;
                } else {
                    return -1;
                }
            } else {
                return -1;
            }
        } else if (src[i] == '+') {
            dest[j++] = ' ';
            i++;
        } else {
            dest[j++] = src[i++];
        }
    }
    
    dest[j] = '\\0';
    return 0;
}

/* Extract target parameter from query string */
int extractTargetParameter(const char* query, char* target, size_t targetSize) {
    if (!query || !target || targetSize == 0) {
        return -1;
    }
    
    if (strlen(query) > MAX_URL_LENGTH) {
        return -1;
    }
    
    const char* pos = strstr(query, "target=");
    if (!pos) {
        return -1;
    }
    
    pos += 7; /* Skip "target=" */
    
    /* Find end of parameter value */
    const char* end = strchr(pos, '&');
    size_t len = end ? (size_t)(end - pos) : strlen(pos);
    
    if (len >= targetSize) {
        return -1;
    }
    
    /* Copy and decode */
    char encoded[MAX_URL_LENGTH];
    if (len >= sizeof(encoded)) {
        return -1;
    }
    
    memcpy(encoded, pos, len);
    encoded[len] = '\\0';
    
    return urlDecode(encoded, target, targetSize);
}

/* Validate if URL matches the allowed pattern */
int isValidRedirect(const char* url) {
    regex_t regex;
    int result;
    
    if (!url || strlen(url) == 0 || strlen(url) > MAX_URL_LENGTH) {
        return 0;
    }
    
    /* Compile regex pattern */
    if (regcomp(&regex, PATTERN, REG_EXTENDED | REG_NOSUB) != 0) {
        return 0;
    }
    
    /* Execute regex match */
    result = regexec(&regex, url, 0, NULL, 0);
    regfree(&regex);
    
    return result == 0 ? 1 : 0;
}

/* Validate and sanitize URL */
void validateAndSanitizeUrl(const char* target, char* result, size_t resultSize) {
    const char* safeDefault = "/";
    
    if (!result || resultSize == 0) {
        return;
    }
    
    if (!target || strlen(target) == 0) {
        strncpy(result, safeDefault, resultSize - 1);
        result[resultSize - 1] = '\\0';
        return;
    }
    
    /* Trim whitespace */
    char normalized[MAX_URL_LENGTH];
    const char* start = target;
    while (isspace((unsigned char)*start)) start++;
    
    size_t len = strlen(start);
    const char* end = start + len - 1;
    while (end > start && isspace((unsigned char)*end)) end--;
    
    len = (size_t)(end - start + 1);
    if (len >= sizeof(normalized)) {
        strncpy(result, safeDefault, resultSize - 1);
        result[resultSize - 1] = '\\0';
        return;
    }
    
    memcpy(normalized, start, len);
    normalized[len] = '\\0';
    
    /* Validate against pattern */
    if (isValidRedirect(normalized)) {
        strncpy(result, normalized, resultSize - 1);
        result[resultSize - 1] = '\\0';
    } else {
        strncpy(result, safeDefault, resultSize - 1);
        result[resultSize - 1] = '\\0';
    }
}

/* Handle HTTP request */
void handleRequest(int clientSocket) {
    char buffer[MAX_REQUEST_SIZE];
    char target[MAX_URL_LENGTH];
    char redirectUrl[MAX_URL_LENGTH];
    char response[MAX_REQUEST_SIZE];
    
    memset(buffer, 0, sizeof(buffer));
    
    /* Read request with size limit */
    ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    
    /* Ensure null termination */
    buffer[bytesRead] = '\\0';
    
    /* Check for GET /redirect */
    if (strncmp(buffer, "GET /redirect", 13) != 0) {
        const char* notFound = "HTTP/1.1 404 Not Found\\r\\n\\r\\n";
        send(clientSocket, notFound, strlen(notFound), 0);
        close(clientSocket);
        return;
    }
    
    /* Extract query string */
    char* queryStart = strchr(buffer, '?');
    target[0] = '\\0';
    
    if (queryStart) {
        char* queryEnd = strchr(queryStart, ' ');
        if (queryEnd) {
            *queryEnd = '\\0';
            extractTargetParameter(queryStart + 1, target, sizeof(target));
        }
    }
    
    /* Validate and get redirect URL */
    validateAndSanitizeUrl(target, redirectUrl, sizeof(redirectUrl));
    
    /* Send redirect response with security headers */
    int responseLen = snprintf(response, sizeof(response),
        "HTTP/1.1 302 Found\\r\\n"
        "Location: %s\\r\\n"
        "X-Content-Type-Options: nosniff\\r\\n"
        "X-Frame-Options: DENY\\r\\n"
        "Content-Length: 0\\r\\n"
        "\\r\\n",
        redirectUrl);
    
    if (responseLen > 0 && (size_t)responseLen < sizeof(response)) {
        send(clientSocket, response, (size_t)responseLen, 0);
    }
    
    close(clientSocket);
}

int main(void) {
    /* Test cases */
    printf("Test Cases:\\n");
    printf("1. Valid: %d\\n", isValidRedirect("https://example.com"));
    printf("2. Valid: %d\\n", isValidRedirect("https://www.example.com/path"));
    printf("3. Valid: %d\\n", isValidRedirect("https://subdomain.example.com/page"));
    printf("4. Invalid: %d\\n", isValidRedirect("https://evil.com"));
    printf("5. Invalid: %d\\n", isValidRedirect("https://evil.com@example.com"));
    
    /* Create socket */
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        return 1;
    }
    
    /* Set socket options to reuse address */
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    /* Bind to port 8000 */
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8000);
    
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to bind socket");
        close(serverSocket);
        return 1;
    }
    
    /* Listen for connections */
    if (listen(serverSocket, 10) < 0) {
        perror("Failed to listen");
        close(serverSocket);
        return 1;
    }
    
    printf("Server started on port 8000\\n");
    
    /* Accept and handle connections */
    while (1) {
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
