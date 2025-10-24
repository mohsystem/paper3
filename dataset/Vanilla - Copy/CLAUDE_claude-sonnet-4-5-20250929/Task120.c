
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Function to sanitize HTML to prevent XSS attacks
char* sanitizeHTML(const char* input) {
    if (input == NULL) {
        char* result = (char*)malloc(1);
        result[0] = '\\0';
        return result;
    }
    
    int len = strlen(input);
    // Allocate enough space (worst case: all characters need escaping)
    char* result = (char*)malloc(len * 6 + 1);
    int j = 0;
    
    for (int i = 0; i < len; i++) {
        switch (input[i]) {
            case '&':
                strcpy(&result[j], "&amp;");
                j += 5;
                break;
            case '<':
                strcpy(&result[j], "&lt;");
                j += 4;
                break;
            case '>':
                strcpy(&result[j], "&gt;");
                j += 4;
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

// Function to generate HTML page with user input
char* generateWebPage(const char* userInput) {
    char* sanitizedInput = sanitizeHTML(userInput);
    
    // Allocate memory for HTML content
    int htmlSize = 1024 + strlen(sanitizedInput);
    char* html = (char*)malloc(htmlSize);
    
    sprintf(html,
        "<!DOCTYPE html>\\n"
        "<html>\\n"
        "<head>\\n"
        "    <title>User Input Display</title>\\n"
        "    <style>\\n"
        "        body { font-family: Arial, sans-serif; margin: 40px; }\\n"
        "        .container { border: 2px solid #333; padding: 20px; border-radius: 5px; }\\n"
        "        h1 { color: #333; }\\n"
        "    </style>\\n"
        "</head>\\n"
        "<body>\\n"
        "    <div class=\\"container\\">\\n"
        "        <h1>User Input Display</h1>\\n"
        "        <p><strong>You entered:</strong></p>\\n"
        "        <p>%s</p>\\n"
        "    </div>\\n"
        "</body>\\n"
        "</html>",
        sanitizedInput
    );
    
    free(sanitizedInput);
    return html;
}

int main() {
    char* result;
    
    printf("Test Case 1: Simple text\\n");
    result = generateWebPage("Hello, World!");
    printf("%s\\n", result);
    free(result);
    printf("\\n================================================================================\\n\\n");
    
    printf("Test Case 2: Text with HTML tags (XSS prevention)\\n");
    result = generateWebPage("<script>alert('XSS')</script>");
    printf("%s\\n", result);
    free(result);
    printf("\\n================================================================================\\n\\n");
    
    printf("Test Case 3: Text with special characters\\n");
    result = generateWebPage("Price: $100 & up, 50% off!");
    printf("%s\\n", result);
    free(result);
    printf("\\n================================================================================\\n\\n");
    
    printf("Test Case 4: Multi-line text\\n");
    result = generateWebPage("Line 1\\nLine 2\\nLine 3");
    printf("%s\\n", result);
    free(result);
    printf("\\n================================================================================\\n\\n");
    
    printf("Test Case 5: Empty input\\n");
    result = generateWebPage("");
    printf("%s\\n", result);
    free(result);
    printf("\\n================================================================================\\n\\n");
    
    return 0;
}
