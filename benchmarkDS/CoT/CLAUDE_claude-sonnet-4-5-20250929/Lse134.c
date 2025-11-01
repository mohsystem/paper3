
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// HTML escape function to prevent XSS attacks
char* escape(const char* input) {
    if (input == NULL) {
        char* empty = (char*)malloc(1);
        empty[0] = '\\0';
        return empty;
    }
    
    size_t len = strlen(input);
    // Allocate buffer (worst case: every char becomes 6 chars like &#x27;)
    char* escaped = (char*)malloc(len * 6 + 1);
    size_t pos = 0;
    
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '<':
                strcpy(&escaped[pos], "&lt;");
                pos += 4;
                break;
            case '>':
                strcpy(&escaped[pos], "&gt;");
                pos += 4;
                break;
            case '&':
                strcpy(&escaped[pos], "&amp;");
                pos += 5;
                break;
            case '"':
                strcpy(&escaped[pos], "&quot;");
                pos += 6;
                break;
            case '\\'':
                strcpy(&escaped[pos], "&#x27;");
                pos += 6;
                break;
            case '/':
                strcpy(&escaped[pos], "&#x2F;");
                pos += 6;
                break;
            default:
                escaped[pos++] = input[i];
        }
    }
    escaped[pos] = '\\0';
    return escaped;
}

// Hello function that returns greeting with escaped username
char* hello(const char* username) {
    char* sanitized = escape(username);
    size_t len = strlen("Hello ") + strlen(sanitized) + 1;
    char* result = (char*)malloc(len);
    
    strcpy(result, "Hello ");
    strcat(result, sanitized);
    
    free(sanitized);
    return result;
}

int main() {
    // Test cases
    printf("Test Cases:\\n");
    
    char* result1 = hello("John");
    printf("1. %s\\n", result1);
    free(result1);
    
    char* result2 = hello("Alice<script>alert('XSS')</script>");
    printf("2. %s\\n", result2);
    free(result2);
    
    char* result3 = hello("Bob&Carol");
    printf("3. %s\\n", result3);
    free(result3);
    
    char* result4 = hello("User\\"123\\"");
    printf("4. %s\\n", result4);
    free(result4);
    
    char* result5 = hello("Test'User");
    printf("5. %s\\n", result5);
    free(result5);
    
    printf("\\nNote: C implementation shows core logic.\\n");
    printf("For web routing, use libraries like libmicrohttpd or write custom HTTP server\\n");
    
    return 0;
}
