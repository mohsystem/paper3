
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>

int isValidHostname(const char* hostname) {
    if (hostname == NULL || strlen(hostname) == 0) {
        return 0;
    }
    
    for (int i = 0; hostname[i] != '\\0'; i++) {
        char c = hostname[i];
        if (!isalnum(c) && c != '.' && c != '-') {
            return 0;
        }
    }
    
    if (strstr(hostname, "..") != NULL) {
        return 0;
    }
    
    return 1;
}

void securePing(const char* url, char* result, size_t result_size) {
    if (url == NULL || strlen(url) == 0) {
        snprintf(result, result_size, "Error: URL parameter is required");
        return;
    }
    
    if (!isValidHostname(url)) {
        snprintf(result, result_size, "Error: Invalid hostname format");
        return;
    }
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    int status = getaddrinfo(url, "80", &hints, &res);
    if (status != 0) {
        snprintf(result, result_size, "Error: Unknown host - %s", url);
        return;
    }
    
    int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd < 0) {
        freeaddrinfo(res);
        snprintf(result, result_size, "Error: Unable to create socket");
        return;
    }
    
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    int reachable = (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0);
    
    close(sockfd);
    freeaddrinfo(res);
    
    snprintf(result, result_size, "Ping result for %s: %s", 
             url, reachable ? "Host is reachable" : "Host found but not reachable on port 80");
}

int main() {
    char result[256];
    
    printf("Test 1: ");
    securePing("google.com", result, sizeof(result));
    printf("%s\\n", result);
    
    printf("Test 2: ");
    securePing("localhost", result, sizeof(result));
    printf("%s\\n", result);
    
    printf("Test 3: ");
    securePing("127.0.0.1", result, sizeof(result));
    printf("%s\\n", result);
    
    printf("Test 4: ");
    securePing("invalid..host", result, sizeof(result));
    printf("%s\\n", result);
    
    printf("Test 5: ");
    securePing("; rm -rf /", result, sizeof(result));
    printf("%s\\n", result);
    
    printf("\\nNote: Full web server implementation requires additional libraries like libmicrohttpd\\n");
    
    return 0;
}
