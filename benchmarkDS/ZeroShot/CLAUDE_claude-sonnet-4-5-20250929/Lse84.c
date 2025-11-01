
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void urlDecode(const char* src, char* dst) {
    while (*src) {
        if (*src == '%' && src[1] && src[2]) {
            int value;
            sscanf(src + 1, "%2x", &value);
            *dst++ = (char)value;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\\0';
}

char* processRequest(const char* payload) {
    static char result[256];
    
    if (payload == NULL || strlen(payload) == 0) {
        strcpy(result, "Error: Missing payload parameter");
        return result;
    }
    
    if (strstr(payload, "type: Create") != NULL || 
        strstr(payload, "type:Create") != NULL) {
        strcpy(result, "Error: Create type is not allowed");
        return result;
    }
    
    strcpy(result, "Success: Payload processed");
    return result;
}

int main() {
    // Test cases
    printf("Test Case 1: %s\\n", processRequest(""));
    printf("Test Case 2: %s\\n", processRequest(NULL));
    printf("Test Case 3: %s\\n", processRequest("type: Create"));
    printf("Test Case 4: %s\\n", processRequest("type: Update"));
    printf("Test Case 5: %s\\n", processRequest("name: test\\nvalue: 123"));
    
    // Start web server
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(5000);
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);
    
    printf("Server started on port 5000\\n");
    
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        char buffer[4096] = {0};
        read(client_fd, buffer, 4096);
        
        char payload[1024] = {0};
        char* pos = strstr(buffer, "payload=");
        if (pos != NULL) {
            char* end = strchr(pos, ' ');
            int len = end ? (end - pos - 8) : strlen(pos + 8);
            strncpy(payload, pos + 8, len);
            payload[len] = '\\0';
            
            char decoded[1024];
            urlDecode(payload, decoded);
            strcpy(payload, decoded);
        }
        
        char* response = processRequest(payload);
        char http_response[2048];
        sprintf(http_response, "HTTP/1.1 200 OK\\r\\nContent-Type: text/plain\\r\\n\\r\\n%s", response);
        
        send(client_fd, http_response, strlen(http_response), 0);
        close(client_fd);
    }
    
    close(server_fd);
    return 0;
}
