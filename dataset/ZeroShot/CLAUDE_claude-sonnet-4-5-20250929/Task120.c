
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// HTML escape function to prevent XSS attacks
char* escape_html(const char* input) {
    if (input == NULL) {
        char* empty = (char*)malloc(1);
        empty[0] = '\\0';
        return empty;
    }
    
    int len = strlen(input);
    // Allocate enough space (worst case: each char becomes 6 chars like &#x27;)
    char* escaped = (char*)malloc(len * 6 + 1);
    int j = 0;
    
    for (int i = 0; i < len; i++) {
        switch (input[i]) {
            case '<':
                strcpy(&escaped[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&escaped[j], "&gt;");
                j += 4;
                break;
            case '&':
                strcpy(&escaped[j], "&amp;");
                j += 5;
                break;
            case '"':
                strcpy(&escaped[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&escaped[j], "&#x27;");
                j += 6;
                break;
            case '/':
                strcpy(&escaped[j], "&#x2F;");
                j += 6;
                break;
            default:
                escaped[j++] = input[i];
        }
    }
    escaped[j] = '\\0';
    return escaped;
}

char* display_user_input(const char* user_input) {
    char* sanitized = escape_html(user_input);
    
    // Allocate memory for HTML output
    int size = strlen(sanitized) + 500;
    char* html = (char*)malloc(size);
    
    snprintf(html, size,
        "<!DOCTYPE html>\\n"
        "<html>\\n"
        "<head>\\n"
        "    <meta charset=\\"UTF-8\\">\\n"
        "    <title>User Input Display</title>\\n"
        "</head>\\n"
        "<body>\\n"
        "    <h1>User Input Display</h1>\\n"
        "    <p>You entered: %s</p>\\n"
        "</body>\\n"
        "</html>",
        sanitized
    );
    
    free(sanitized);
    return html;
}

int main() {
    char* result;
    
    printf("=== Test Case 1: Normal Text ===\\n");
    const char* test1 = "Hello, World!";
    result = display_user_input(test1);
    printf("%s\\n\\n", result);
    free(result);
    
    printf("=== Test Case 2: XSS Attack Attempt (Script Tag) ===\\n");
    const char* test2 = "<script>alert('XSS')</script>";
    result = display_user_input(test2);
    printf("%s\\n\\n", result);
    free(result);
    
    printf("=== Test Case 3: Special Characters ===\\n");
    const char* test3 = "Price: $100 & Free Shipping!";
    result = display_user_input(test3);
    printf("%s\\n\\n", result);
    free(result);
    
    printf("=== Test Case 4: HTML Injection Attempt ===\\n");
    const char* test4 = "<img src=x onerror=alert('XSS')>";
    result = display_user_input(test4);
    printf("%s\\n\\n", result);
    free(result);
    
    printf("=== Test Case 5: Mixed Content ===\\n");
    const char* test5 = "My email: user@example.com & phone: 555-1234";
    result = display_user_input(test5);
    printf("%s\\n", result);
    free(result);
    
    return 0;
}
