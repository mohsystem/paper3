
// C - SECURE VERSION with input validation
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_OUTPUT 2048

int is_valid_char(char c) {
    return isalnum(c) || c == '.' || c == '-';
}

int is_valid_host(const char* url) {
    if (url == NULL || strlen(url) == 0 || strlen(url) > 255) {
        return 0;
    }
    
    for (int i = 0; url[i] != '\\0'; i++) {
        if (!is_valid_char(url[i])) {
            return 0;
        }
    }
    
    // Check for consecutive dots or dashes
    for (int i = 0; url[i] != '\\0'; i++) {
        if ((url[i] == '.' || url[i] == '-') && 
            (url[i+1] == '.' || url[i+1] == '-')) {
            return 0;
        }
    }
    
    return 1;
}

int check_connection(const char* host, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;
    
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
    
    struct hostent* server = gethostbyname(host);
    if (server == NULL) {
        close(sock);
        return 0;
    }
    
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    
    int result = connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0;
    close(sock);
    return result;
}

char* ping(const char* url) {
    char* output = (char*)malloc(MAX_OUTPUT);
    if (output == NULL) return NULL;
    
    memset(output, 0, MAX_OUTPUT);
    
    if (url == NULL || strlen(url) == 0) {
        snprintf(output, MAX_OUTPUT, "Error: URL cannot be empty");
        return output;
    }
    
    if (!is_valid_host(url)) {
        snprintf(output, MAX_OUTPUT, "Error: Invalid hostname or IP address format");
        return output;
    }
    
    snprintf(output, MAX_OUTPUT, "Ping test for: %s\\n", url);
    
    struct hostent* host = gethostbyname(url);
    if (host == NULL) {
        char temp[256];
        snprintf(temp, sizeof(temp), "Error: Unknown host - %s", url);
        strncat(output, temp, MAX_OUTPUT - strlen(output) - 1);
        return output;
    }
    
    struct in_addr** addr_list = (struct in_addr**)host->h_addr_list;
    if (addr_list[0] != NULL) {
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, addr_list[0], ip, INET_ADDRSTRLEN);
        
        char temp[256];
        snprintf(temp, sizeof(temp), "Host: %s\\n", ip);
        strncat(output, temp, MAX_OUTPUT - strlen(output) - 1);
        
        int ports[] = {80, 443, 22};
        int reachable = 0;
        
        for (int i = 0; i < 3; i++) {
            if (check_connection(url, ports[i])) {
                snprintf(temp, sizeof(temp), "Port %d: Open\\n", ports[i]);
                strncat(output, temp, MAX_OUTPUT - strlen(output) - 1);
                reachable = 1;
                break;
            }
        }
        
        snprintf(temp, sizeof(temp), "Reachable: %s\\n", reachable ? "Yes" : "No");
        strncat(output, temp, MAX_OUTPUT - strlen(output) - 1);
    }
    
    return output;
}

int main() {
    char* result;
    
    printf("=== Test Case 1: Valid domain ===\\n");
    result = ping("google.com");
    printf("%s\\n", result);
    free(result);
    
    printf("=== Test Case 2: Valid IP ===\\n");
    result = ping("8.8.8.8");
    printf("%s\\n", result);
    free(result);
    
    printf("=== Test Case 3: Localhost ===\\n");
    result = ping("127.0.0.1");
    printf("%s\\n", result);
    free(result);
    
    printf("=== Test Case 4: Invalid input (command injection attempt) ===\\n");
    result = ping("google.com; rm -rf /");
    printf("%s\\n", result);
    free(result);
    
    printf("=== Test Case 5: Empty input ===\\n");
    result = ping("");
    printf("%s\\n", result);
    free(result);
    
    return 0;
}
