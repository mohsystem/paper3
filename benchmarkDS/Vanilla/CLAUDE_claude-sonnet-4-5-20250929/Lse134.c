
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char* escapeHtml(const char* input) {
    if (input == NULL) {
        return NULL;
    }
    
    size_t len = strlen(input);
    size_t newLen = len;
    
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '<':
            case '>':
                newLen += 3;
                break;
            case '&':
                newLen += 4;
                break;
            case '"':
                newLen += 5;
                break;
            case '\\'':
            case '/':
                newLen += 5;
                break;
        }
    }
    
    char* result = (char*)malloc(newLen + 1);
    if (result == NULL) {
        return NULL;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '<':
                strcpy(&result[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&result[j], "&gt;");
                j += 4;
                break;
            case '&':
                strcpy(&result[j], "&amp;");
                j += 5;
                break;
            case '"':
                strcpy(&result[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&result[j], "&#x27;");
                j += 6;
                break;
            case '/':
                strcpy(&result[j], "&#x2F;");
                j += 6;
                break;
            default:
                result[j++] = input[i];
        }
    }
    result[j] = '\\0';
    return result;
}

char* hello(const char* username) {
    char* escaped = escapeHtml(username);
    if (escaped == NULL) {
        return NULL;
    }
    
    size_t resultLen = strlen("Hello ") + strlen(escaped) + 1;
    char* result = (char*)malloc(resultLen);
    if (result == NULL) {
        free(escaped);
        return NULL;
    }
    
    strcpy(result, "Hello ");
    strcat(result, escaped);
    free(escaped);
    
    return result;
}

int main() {
    // Test cases
    char* result1 = hello("John");
    printf("%s\\n", result1);
    free(result1);
    
    char* result2 = hello("Alice");
    printf("%s\\n", result2);
    free(result2);
    
    char* result3 = hello("<script>alert('XSS')</script>");
    printf("%s\\n", result3);
    free(result3);
    
    char* result4 = hello("User&Name");
    printf("%s\\n", result4);
    free(result4);
    
    char* result5 = hello("Test'User\\"123");
    printf("%s\\n", result5);
    free(result5);
    
    return 0;
}
