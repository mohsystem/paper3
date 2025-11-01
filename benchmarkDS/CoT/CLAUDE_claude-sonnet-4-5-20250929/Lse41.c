
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <regex.h>

typedef struct {
    char username[256];
    char email[256];
    char ssnLast4[5];
    char sessionToken[128];
} User;

void escapeHtml(const char* input, char* output, size_t maxLen) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < maxLen - 6; i++) {
        switch (input[i]) {
            case '&': j += snprintf(output + j, maxLen - j, "&amp;"); break;
            case '<': j += snprintf(output + j, maxLen - j, "&lt;"); break;
            case '>': j += snprintf(output + j, maxLen - j, "&gt;"); break;
            case '"': j += snprintf(output + j, maxLen - j, "&quot;"); break;
            case '\\'': j += snprintf(output + j, maxLen - j, "&#x27;"); break;
            default: output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

int validateSSN(const char* ssn) {
    regex_t regex;
    int result;
    regcomp(&regex, "^[0-9][0-9][0-9][0-9]$", REG_EXTENDED);
    result = regexec(&regex, ssn, 0, NULL, 0);
    regfree(&regex);
    return result == 0;
}

User* authenticateUser(const char* token) {
    // Simulated user database - in production, use secure database
    if (strcmp(token, "valid_token_123") == 0) {
        User* user = (User*)malloc(sizeof(User));
        escapeHtml("john_doe", user->username, sizeof(user->username));
        escapeHtml("john@example.com", user->email, sizeof(user->email));
        
        if (validateSSN("5678")) {
            strcpy(user->ssnLast4, "5678");
        } else {
            strcpy(user->ssnLast4, "****");
        }
        strcpy(user->sessionToken, token);
        return user;
    }
    return NULL;
}

void extractHeader(const char* request, const char* headerName, char* result, size_t maxLen) {
    const char* pos = strstr(request, headerName);
    if (pos != NULL) {
        pos += strlen(headerName);
        const char* end = strstr(pos, "\\r\\n");
        if (end != NULL) {
            size_t len = end - pos;
            if (len < maxLen) {
                strncpy(result, pos, len);
                result[len] = '\\0';
                return;
            }
        }
    }
    result[0] = '\\0';
}

void handleRequest(int clientSocket) {
    char buffer[4096] = {0};
    read(clientSocket, buffer, sizeof(buffer));
    
    char authHeader[256] = {0};
    extractHeader(buffer, "Authorization: ", authHeader, sizeof(authHeader));
    
    User* user = authenticateUser(authHeader);
    char response[8192];
    
    if (user == NULL) {
        snprintf(response, sizeof(response),
                "HTTP/1.1 401 Unauthorized\\r\\n"
                "Content-Type: text/html; charset=utf-8\\r\\n"
                "X-Content-Type-Options: nosniff\\r\\n"
                "X-Frame-Options: DENY\\r\\n\\r\\n"
                "<html><body><h1>Unauthorized</h1></body></html>");
    } else {
        snprintf(response, sizeof(response),
                "HTTP/1.1 200 OK\\r\\n"
                "Content-Type: text/html; charset=utf-8\\r\\n"
                "X-Content-Type-Options: nosniff\\r\\n"
                "X-Frame-Options: DENY\\r\\n\\r\\n"
                "<html><head><meta charset='UTF-8'></head><body>"
                "<h1>User Information</h1>"
                "<p>Username: %s</p>"
                "<p>Email: %s</p>"
                "<p>SSN Last 4: ****-%s</p>"
                "</body></html>",
                user->username, user->email, user->ssnLast4);
        free(user);
    }
    
    write(clientSocket, response, strlen(response));
    close(clientSocket);
}

int main() {
    printf("Testing with 5 scenarios:\\n");
    printf("1. Valid token: curl -H 'Authorization: valid_token_123' http://localhost:8080/info\\n");
    printf("2. Invalid token: curl -H 'Authorization: invalid' http://localhost:8080/info\\n");
    printf("3. No token: curl http://localhost:8080/info\\n");
    printf("4. XSS attempt in username (blocked by escapeHtml)\\n");
    printf("5. Valid user data display with masked SSN\\n");
    
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    
    if (bind(serverSocket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(serverSocket);
        return 1;
    }
    
    listen(serverSocket, 3);
    printf("Server started on port 8080. Press Ctrl+C to stop.\\n");
    
    while (1) {
        int clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket >= 0) {
            handleRequest(clientSocket);
        }
    }
    
    close(serverSocket);
    return 0;
}
