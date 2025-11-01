
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void urlDecode(const char* src, char* dest, size_t destSize) {
    size_t i = 0, j = 0;
    while (src[i] && j < destSize - 1) {
        if (src[i] == '%' && src[i+1] && src[i+2]) {
            int value;
            sscanf(&src[i+1], "%2x", &value);
            dest[j++] = (char)value;
            i += 3;
        } else if (src[i] == '+') {
            dest[j++] = ' ';
            i++;
        } else {
            dest[j++] = src[i++];
        }
    }
    dest[j] = '\\0';
}

void sanitizeInput(char* input) {
    size_t i, j = 0;
    for (i = 0; input[i] && j < 100; i++) {
        if (!iscntrl(input[i])) {
            input[j++] = input[i];
        }
    }
    input[j] = '\\0';
}

void escapeHtml(const char* input, char* output, size_t outputSize) {
    size_t i = 0, j = 0;
    while (input[i] && j < outputSize - 10) {
        switch (input[i]) {
            case '&': strcpy(&output[j], "&amp;"); j += 5; break;
            case '<': strcpy(&output[j], "&lt;"); j += 4; break;
            case '>': strcpy(&output[j], "&gt;"); j += 4; break;
            case '"': strcpy(&output[j], "&quot;"); j += 6; break;
            case '\\'': strcpy(&output[j], "&#x27;"); j += 6; break;
            default: output[j++] = input[i];
        }
        i++;
    }
    output[j] = '\\0';
}

void httpHelloPage(FILE* request, FILE* response) {
    char line[1024];
    char body[1024] = {0};
    char username[256] = "Guest";
    char decodedUsername[256];
    char escapedUsername[512];
    
    // Read headers
    while (fgets(line, sizeof(line), request)) {
        if (strcmp(line, "\\r\\n") == 0 || strcmp(line, "\\n") == 0) {
            break;
        }
    }
    
    // Read body
    if (fgets(body, sizeof(body), request)) {
        char* usernamePos = strstr(body, "username=");
        if (usernamePos) {
            usernamePos += 9;
            char* end = strchr(usernamePos, '&');
            if (end) *end = '\\0';
            
            urlDecode(usernamePos, decodedUsername, sizeof(decodedUsername));
            sanitizeInput(decodedUsername);
            
            if (strlen(decodedUsername) > 0) {
                strncpy(username, decodedUsername, sizeof(username) - 1);
            }
        }
    }
    
    escapeHtml(username, escapedUsername, sizeof(escapedUsername));
    
    // Write response
    fprintf(response, "HTTP/1.1 200 OK\\r\\n");
    fprintf(response, "Content-Type: text/html\\r\\n");
    fprintf(response, "\\r\\n");
    fprintf(response, "<html><body>\\r\\n");
    fprintf(response, "<h1>Hello %s</h1>\\r\\n", escapedUsername);
    fprintf(response, "</body></html>\\r\\n");
    fflush(response);
}

int main() {
    printf("Test cases for httpHelloPage:\\n");
    
    const char* testCases[] = {
        "username=John",
        "username=Alice%20Smith",
        "username=%3Cscript%3Ealert%28%29%3C%2Fscript%3E",
        "username=",
        "username=Bob&other=data"
    };
    
    for (int i = 0; i < 5; i++) {
        printf("\\nTest Case %d:\\n", i + 1);
        
        char requestStr[1024];
        snprintf(requestStr, sizeof(requestStr), "POST / HTTP/1.1\\r\\n\\r\\n%s", testCases[i]);
        
        FILE* request = fmemopen(requestStr, strlen(requestStr), "r");
        FILE* response = fopen("/dev/stdout", "w");
        
        if (request && response) {
            httpHelloPage(request, response);
            fclose(request);
        }
        printf("\\n");
    }
    
    return 0;
}
