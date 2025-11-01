
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_PAYLOAD_SIZE 10000
#define PORT 8080
#define BUFFER_SIZE 30000

char* urlDecode(const char* str) {
    int len = strlen(str);
    char* result = (char*)malloc(len + 1);
    int j = 0;
    
    for (int i = 0; i < len; i++) {
        if (str[i] == '%' && i + 2 < len && isxdigit(str[i+1]) && isxdigit(str[i+2])) {
            char hex[3] = {str[i+1], str[i+2], '\\0'};
            result[j++] = (char)strtol(hex, NULL, 16);
            i += 2;
        } else if (str[i] == '+') {
            result[j++] = ' ';
        } else {
            result[j++] = str[i];
        }
    }
    result[j] = '\\0';
    return result;
}

char* extractPayload(const char* query) {
    const char* payloadStart = strstr(query, "payload=");
    if (!payloadStart) return NULL;
    
    payloadStart += 8;
    const char* payloadEnd = strchr(payloadStart, '&');
    
    int len = payloadEnd ? (payloadEnd - payloadStart) : strlen(payloadStart);
    char* encoded = (char*)malloc(len + 1);
    strncpy(encoded, payloadStart, len);
    encoded[len] = '\\0';
    
    char* decoded = urlDecode(encoded);
    free(encoded);
    return decoded;
}

char* extractJsonValue(const char* json, const char* key) {
    char pattern[256];
    snprintf(pattern, sizeof(pattern), "\\"%s\\"", key);
    
    const char* keyPos = strstr(json, pattern);
    if (!keyPos) return NULL;
    
    const char* colonPos = strchr(keyPos, ':');
    if (!colonPos) return NULL;
    
    while (*colonPos && isspace(*colonPos)) colonPos++;
    colonPos++;
    while (*colonPos && isspace(*colonPos)) colonPos++;
    
    if (*colonPos != '"') return NULL;
    colonPos++;
    
    const char* endQuote = strchr(colonPos, '"');
    if (!endQuote) return NULL;
    
    int len = endQuote - colonPos;
    char* value = (char*)malloc(len + 1);
    strncpy(value, colonPos, len);
    value[len] = '\\0';
    
    return value;
}

int isValidJson(const char* str) {
    int braceCount = 0;
    int inString = 0;
    
    for (int i = 0; str[i]; i++) {
        if (str[i] == '"' && (i == 0 || str[i-1] != '\\\\')) {\n            inString = !inString;\n        }\n        if (!inString) {\n            if (str[i] == '{') braceCount++;\n            if (str[i] == '}') braceCount--;\n        }\n    }\n    return braceCount == 0;\n}\n\nchar* handleRequest(const char* request) {\n    char* response = (char*)malloc(BUFFER_SIZE);\n    \n    if (strstr(request, "GET /api?") == NULL) {\n        snprintf(response, BUFFER_SIZE, \n                "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Invalid request\\"}");\n        return response;\n    }\n    \n    const char* queryStart = strchr(request, '?');
    if (!queryStart) {
        snprintf(response, BUFFER_SIZE,
                "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Missing query\\"}");
        return response;
    }
    
    char* payload = extractPayload(queryStart + 1);
    
    if (!payload) {
        snprintf(response, BUFFER_SIZE,
                "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Missing payload parameter\\"}");
        return response;
    }
    
    if (strlen(payload) > MAX_PAYLOAD_SIZE) {
        snprintf(response, BUFFER_SIZE,
                "HTTP/1.1 413 Payload Too Large\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Payload too large\\"}");
        free(payload);
        return response;
    }
    
    if (!isValidJson(payload)) {
        snprintf(response, BUFFER_SIZE,
                "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Invalid JSON format\\"}");
        free(payload);
        return response;
    }
    
    char* name = extractJsonValue(payload, "name");
    free(payload);
    
    if (!name) {
        snprintf(response, BUFFER_SIZE,
                "HTTP/1.1 400 Bad Request\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"error\\":\\"Name field not found\\"}");
        return response;
    }
    
    snprintf(response, BUFFER_SIZE,
            "HTTP/1.1 200 OK\\r\\nContent-Type: application/json\\r\\n\\r\\n{\\"name\\":\\"%s\\"}", name);
    free(name);
    
    return response;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    printf("Test Cases:\\n");
    printf("1. Valid: http://localhost:8080/api?payload={\\"name\\":\\"John\\"}\\n");
    printf("2. Valid: http://localhost:8080/api?payload={\\"name\\":\\"Alice\\",\\"age\\":30}\\n");
    printf("3. Missing name: http://localhost:8080/api?payload={\\"age\\":25}\\n");
    printf("4. Invalid JSON: http://localhost:8080/api?payload={invalid}\\n");
    printf("5. Missing payload: http://localhost:8080/api\\n\\n");
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Server started on port %d\\n", PORT);
    
    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }
        
        char* buffer = (char*)malloc(BUFFER_SIZE);
        memset(buffer, 0, BUFFER_SIZE);
        read(new_socket, buffer, BUFFER_SIZE);
        
        char* response = handleRequest(buffer);
        
        send(new_socket, response, strlen(response), 0);
        
        free(buffer);
        free(response);
        close(new_socket);
    }
    
    return 0;
}
