
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <regex.h>

#define TIMEOUT 5
#define MAX_RESPONSE_SIZE 1048576
#define BUFFER_SIZE 4096

int is_private_or_local_address(const char* host) {
    if (strcmp(host, "localhost") == 0 || 
        strcmp(host, "127.0.0.1") == 0 || 
        strcmp(host, "0.0.0.0") == 0) {
        return 1;
    }
    
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    
    if (getaddrinfo(host, NULL, &hints, &res) == 0) {
        struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
        unsigned char* ip = (unsigned char*)&addr->sin_addr.s_addr;
        
        int is_private = (ip[0] == 10) ||
                        (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) ||
                        (ip[0] == 192 && ip[1] == 168) ||
                        (ip[0] == 127) ||
                        (ip[0] == 169 && ip[1] == 254);
        
        freeaddrinfo(res);
        return is_private;
    }
    
    return 0;
}

char* make_http_request(const char* url_string) {
    static char result[MAX_RESPONSE_SIZE];
    
    if (url_string == NULL || strlen(url_string) == 0) {
        strcpy(result, "Error: URL cannot be empty");
        return result;
    }
    
    // Simple URL parsing
    regex_t regex;
    regmatch_t matches[5];
    int ret = regcomp(&regex, "^(https?)://([^:/]+)(:([0-9]+))?(/.*)?$", REG_EXTENDED);
    
    if (ret != 0) {
        strcpy(result, "Error: Failed to compile regex");
        return result;
    }
    
    ret = regexec(&regex, url_string, 5, matches, 0);
    regfree(&regex);
    
    if (ret != 0) {
        strcpy(result, "Error: Invalid URL format");
        return result;
    }
    
    // Extract protocol and host
    char protocol[10], host[256], path[512];
    int port = 80;
    
    int proto_len = matches[1].rm_eo - matches[1].rm_so;
    strncpy(protocol, url_string + matches[1].rm_so, proto_len);
    protocol[proto_len] = '\\0';
    
    int host_len = matches[2].rm_eo - matches[2].rm_so;
    strncpy(host, url_string + matches[2].rm_so, host_len);
    host[host_len] = '\\0';
    
    if (matches[4].rm_so != -1) {
        char port_str[10];
        int port_len = matches[4].rm_eo - matches[4].rm_so;
        strncpy(port_str, url_string + matches[4].rm_so, port_len);
        port_str[port_len] = '\\0';
        port = atoi(port_str);
    }
    
    strcpy(path, matches[4].rm_so == -1 ? "/" : url_string + matches[4].rm_so);
    
    // Security checks
    if (strcmp(protocol, "http") != 0 && strcmp(protocol, "https") != 0) {
        strcpy(result, "Error: Only HTTP and HTTPS protocols are allowed");
        return result;
    }
    
    if (is_private_or_local_address(host)) {
        strcpy(result, "Error: Requests to private/internal addresses are not allowed");
        return result;
    }
    
    // Create socket and connect
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        strcpy(result, "Error: Failed to create socket");
        return result;
    }
    
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    
    struct hostent* server = gethostbyname(host);
    if (server == NULL) {
        close(sock);
        strcpy(result, "Error: Failed to resolve hostname");
        return result;
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sock);
        strcpy(result, "Error: Failed to connect");
        return result;
    }
    
    // Send request
    char request[1024];
    snprintf(request, sizeof(request),
             "GET %s HTTP/1.1\\r\\nHost: %s\\r\\nUser-Agent: SecureHttpClient/1.0\\r\\nConnection: close\\r\\n\\r\\n",
             path, host);
    
    if (send(sock, request, strlen(request), 0) < 0) {
        close(sock);
        strcpy(result, "Error: Failed to send request");
        return result;
    }
    
    // Read response
    char buffer[BUFFER_SIZE];
    int total_bytes = 0;
    int bytes_read;
    result[0] = '\\0';
    
    while ((bytes_read = recv(sock, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        total_bytes += bytes_read;
        if (total_bytes > MAX_RESPONSE_SIZE) {
            close(sock);
            strcpy(result, "Error: Response size exceeds limit");
            return result;
        }
        buffer[bytes_read] = '\\0';
        strncat(result, buffer, MAX_RESPONSE_SIZE - strlen(result) - 1);
    }
    
    close(sock);
    return result;
}

int main() {
    printf("=== Test Case 1: Valid HTTP URL ===\\n");
    printf("%s\\n", make_http_request("http://www.example.com"));
    
    printf("\\n=== Test Case 2: Invalid Protocol ===\\n");
    printf("%s\\n", make_http_request("ftp://example.com"));
    
    printf("\\n=== Test Case 3: Localhost (blocked) ===\\n");
    printf("%s\\n", make_http_request("http://localhost:8080"));
    
    printf("\\n=== Test Case 4: Private IP (blocked) ===\\n");
    printf("%s\\n", make_http_request("http://192.168.1.1"));
    
    printf("\\n=== Test Case 5: Empty URL ===\\n");
    printf("%s\\n", make_http_request(""));
    
    return 0;
}
