
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* htmlEscape(const char* input) {
    if (input == NULL) return strdup("");
    
    size_t len = strlen(input);
    char* output = (char*)malloc(len * 6 + 1);
    if (output == NULL) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&output[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&output[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&output[j], "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(&output[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&output[j], "&#x27;");
                j += 6;
                break;
            case '/':
                strcpy(&output[j], "&#x2F;");
                j += 6;
                break;
            default:
                output[j++] = input[i];
        }
    }
    output[j] = '\\0';
    return output;
}

void http_hello_page(FILE* requestBody, FILE* response) {
    char username[8193] = {0};
    char outputbuf[16384] = {0};
    
    // Read username from request body with size limit
    size_t bytesRead = fread(username, 1, 8192, requestBody);
    username[bytesRead] = '\\0';
    
    // Trim and validate
    char* start = username;
    while (*start == ' ' || *start == '\\t' || *start == '\\n' || *start == '\\r') start++;
    
    size_t len = strlen(start);
    while (len > 0 && (start[len-1] == ' ' || start[len-1] == '\\t' || 
                       start[len-1] == '\\n' || start[len-1] == '\\r')) {
        start[--len] = '\\0';
    }
    
    if (len > 100) {
        start[100] = '\\0';
    }
    
    // Sanitize username
    char* escapedUsername = htmlEscape(start);
    if (escapedUsername == NULL) {
        escapedUsername = strdup("");
    }
    
    // Construct HTML response
    snprintf(outputbuf, sizeof(outputbuf),
        "HTTP/1.1 200 OK\\r\\n"
        "Content-Type: text/html; charset=UTF-8\\r\\n"
        "X-Content-Type-Options: nosniff\\r\\n"
        "X-Frame-Options: DENY\\r\\n"
        "\\r\\n"
        "<!DOCTYPE html><html><head><meta charset=\\"UTF-8\\">"
        "<title>Hello Page</title></head><body>"
        "<h1>Hello, %s!</h1>"
        "</body></html>",
        escapedUsername);
    
    // Write response
    fwrite(outputbuf, 1, strlen(outputbuf), response);
    fflush(response);
    
    free(escapedUsername);
    fclose(response);
}

int main() {
    // Test cases
    const char* testUsernames[] = {
        "Alice",
        "Bob<script>alert('xss')</script>",
        "Charlie&Sons",
        "Diana\\"O'Neil\\"",
        ""
    };
    
    for (int i = 0; i < 5; i++) {
        printf("Testing with username: %s\\n", testUsernames[i]);
        
        FILE* input = fmemopen((void*)testUsernames[i], strlen(testUsernames[i]), "r");
        char* outputBuffer = NULL;
        size_t outputSize = 0;
        FILE* output = open_memstream(&outputBuffer, &outputSize);
        
        if (input && output) {
            http_hello_page(input, output);
            fclose(input);
            
            if (outputBuffer) {
                printf("%s\\n", outputBuffer);
                free(outputBuffer);
            }
        }
        printf("---\\n");
    }
    
    return 0;
}
