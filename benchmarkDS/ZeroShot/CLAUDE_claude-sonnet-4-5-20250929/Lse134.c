
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* urlDecode(const char* str) {
    int len = strlen(str);
    char* result = (char*)malloc(len + 1);
    int j = 0;
    
    for (int i = 0; i < len; i++) {
        if (str[i] == '%' && i + 2 < len) {
            int value;
            sscanf(&str[i + 1], "%2x", &value);
            result[j++] = (char)value;
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

char* escapeHtml(const char* input) {
    int len = strlen(input);
    char* output = (char*)malloc(len * 6 + 1);
    int j = 0;
    
    for (int i = 0; i < len; i++) {
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
                break;
        }
    }
    output[j] = '\\0';
    return output;
}

char* hello(const char* username) {
    char* safeUsername = escapeHtml(username);
    char* result = (char*)malloc(strlen(safeUsername) + 7);
    sprintf(result, "Hello %s", safeUsername);
    free(safeUsername);
    return result;
}

char* handleRequest(const char* path) {
    if (strncmp(path, "/hello/", 7) == 0) {
        const char* username = path + 7;
        char* decoded = urlDecode(username);
        char* result = hello(decoded);
        free(decoded);
        return result;
    }
    char* result = (char*)malloc(10);
    strcpy(result, "Not Found");
    return result;
}

int main() {
    char* result;
    
    printf("Test 1: ");
    result = handleRequest("/hello/John");
    printf("%s\\n", result);
    free(result);
    
    printf("Test 2: ");
    result = handleRequest("/hello/Alice");
    printf("%s\\n", result);
    free(result);
    
    printf("Test 3: ");
    result = handleRequest("/hello/<script>alert('XSS')</script>");
    printf("%s\\n", result);
    free(result);
    
    printf("Test 4: ");
    result = handleRequest("/hello/User&Name");
    printf("%s\\n", result);
    free(result);
    
    printf("Test 5: ");
    result = handleRequest("/hello/Test%20User");
    printf("%s\\n", result);
    free(result);
    
    return 0;
}
