
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to safely encode user input for HTML display
char* encode_for_html(const char* input) {
    if (input == NULL) {
        char* empty = (char*)malloc(1);
        empty[0] = '\\0';
        return empty;
    }
    
    size_t input_len = strlen(input);
    // Worst case: each character becomes 6 characters (e.g., & -> &amp;)
    char* encoded = (char*)malloc(input_len * 6 + 1);
    if (encoded == NULL) {
        return NULL;
    }
    
    size_t j = 0;
    for (size_t i = 0; i < input_len; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&encoded[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&encoded[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&encoded[j], "&gt;");
                j += 4;
                break;
            case '"':
                strcpy(&encoded[j], "&quot;");
                j += 6;
                break;
            case '\\'':
                strcpy(&encoded[j], "&#x27;");
                j += 6;
                break;
            case '/':
                strcpy(&encoded[j], "&#x2F;");
                j += 6;
                break;
            default:
                encoded[j++] = input[i];
                break;
        }
    }
    encoded[j] = '\\0';
    return encoded;
}

// Function to display user input safely
char* display_user_input(const char* user_input) {
    char* safe_input = encode_for_html(user_input);
    if (safe_input == NULL) {
        return NULL;
    }
    
    size_t html_size = strlen(safe_input) + 256;
    char* html = (char*)malloc(html_size);
    if (html == NULL) {
        free(safe_input);
        return NULL;
    }
    
    snprintf(html, html_size,
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
        safe_input);
    
    free(safe_input);
    return html;
}

int main() {
    char* result;
    
    printf("Test Case 1 - Normal input:\\n");
    result = display_user_input("Hello World");
    if (result) {
        printf("%s\\n\\n", result);
        free(result);
    }
    
    printf("Test Case 2 - Input with HTML tags:\\n");
    result = display_user_input("<script>alert('XSS')</script>");
    if (result) {
        printf("%s\\n\\n", result);
        free(result);
    }
    
    printf("Test Case 3 - Input with special characters:\\n");
    result = display_user_input("Tom & Jerry's \\"Adventure\\"");
    if (result) {
        printf("%s\\n\\n", result);
        free(result);
    }
    
    printf("Test Case 4 - Input with single quotes:\\n");
    result = display_user_input("It's a beautiful day");
    if (result) {
        printf("%s\\n\\n", result);
        free(result);
    }
    
    printf("Test Case 5 - Input with mixed content:\\n");
    result = display_user_input("<div>Hello</div> & <span>World</span>");
    if (result) {
        printf("%s\\n", result);
        free(result);
    }
    
    return 0;
}
