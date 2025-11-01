
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERNAME_LENGTH 256
#define MAX_OUTPUT_LENGTH 1024

typedef struct {
    char username[MAX_USERNAME_LENGTH];
    char outputbuf[MAX_OUTPUT_LENGTH];
} Lse126;

void sanitizeInput(const char* input, char* output, size_t outputSize) {
    size_t j = 0;
    for (size_t i = 0; input[i] != '\\0' && j < outputSize - 6; i++) {
        if (input[i] == '&') {
            strcpy(&output[j], "&amp;");
            j += 5;
        } else if (input[i] == '<') {
            strcpy(&output[j], "&lt;");
            j += 4;
        } else if (input[i] == '>') {
            strcpy(&output[j], "&gt;");
            j += 4;
        } else if (input[i] == '"') {
            strcpy(&output[j], "&quot;");
            j += 6;
        } else if (input[i] == '\\'') {
            strcpy(&output[j], "&#x27;");
            j += 6;
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\\0';
}

void httpHelloPage(const char* username, char* outputbuf, size_t bufferSize) {
    char sanitizedUsername[MAX_USERNAME_LENGTH * 6];
    sanitizeInput(username, sanitizedUsername, sizeof(sanitizedUsername));
    
    snprintf(outputbuf, bufferSize,
        "<!DOCTYPE html>\\n"
        "<html>\\n"
        "<head>\\n"
        "    <title>Hello Page</title>\\n"
        "</head>\\n"
        "<body>\\n"
        "    <h1>Hello, %s!</h1>\\n"
        "</body>\\n"
        "</html>",
        sanitizedUsername);
}

int main() {
    char outputbuf[MAX_OUTPUT_LENGTH];
    
    printf("Test Case 1: Normal username\\n");
    httpHelloPage("John", outputbuf, sizeof(outputbuf));
    printf("%s\\n\\n", outputbuf);
    
    printf("Test Case 2: Username with special characters\\n");
    httpHelloPage("Alice & Bob", outputbuf, sizeof(outputbuf));
    printf("%s\\n\\n", outputbuf);
    
    printf("Test Case 3: Username with HTML tags (XSS attempt)\\n");
    httpHelloPage("<script>alert('XSS')</script>", outputbuf, sizeof(outputbuf));
    printf("%s\\n\\n", outputbuf);
    
    printf("Test Case 4: Username with quotes\\n");
    httpHelloPage("O'Brien", outputbuf, sizeof(outputbuf));
    printf("%s\\n\\n", outputbuf);
    
    printf("Test Case 5: Empty username\\n");
    httpHelloPage("", outputbuf, sizeof(outputbuf));
    printf("%s\\n", outputbuf);
    
    return 0;
}
