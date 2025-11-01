
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// HTML escape function for security
char* escapeHtml(const char* input) {
    if (input == NULL) return NULL;
    
    size_t len = strlen(input);
    size_t newLen = 0;
    
    // Calculate new length
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '&':  newLen += 5; break;  // &amp;
            case '<':  newLen += 4; break;  // &lt;
            case '>':  newLen += 4; break;  // &gt;
            case '"':  newLen += 6; break;  // &quot;
            case '\\'': newLen += 6; break;  // &#x27;
            default:   newLen += 1; break;
        }
    }
    
    char* output = (char*)malloc(newLen + 1);
    if (output == NULL) return NULL;
    
    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        switch (input[i]) {
            case '&':  strcpy(&output[j], "&amp;");  j += 5; break;
            case '<':  strcpy(&output[j], "&lt;");   j += 4; break;
            case '>':  strcpy(&output[j], "&gt;");   j += 4; break;
            case '"':  strcpy(&output[j], "&quot;"); j += 6; break;
            case '\\'': strcpy(&output[j], "&#x27;"); j += 6; break;
            default:   output[j++] = input[i]; break;
        }
    }
    output[j] = '\\0';
    
    return output;
}

// Simulated template rendering
char* renderTemplate(const char* templateName, const char* username) {
    char* safeUsername = escapeHtml(username);
    if (safeUsername == NULL) return NULL;
    
    size_t bufferSize = 512 + strlen(safeUsername);
    char* result = (char*)malloc(bufferSize);
    
    if (result != NULL) {
        snprintf(result, bufferSize,
            "<!DOCTYPE html>\\n"
            "<html>\\n"
            "<head>\\n"
            "    <title>Hello Page</title>\\n"
            "</head>\\n"
            "<body>\\n"
            "    <h1>Hello, %s!</h1>\\n"
            "    <p>Welcome to our secure application.</p>\\n"
            "</body>\\n"
            "</html>",
            safeUsername);
    }
    
    free(safeUsername);
    return result;
}

// Handler function for /hello/<username>
char* hello(const char* username) {
    return renderTemplate("hello.html", username);
}

int main() {
    printf("Testing hello function with 5 test cases:\\n\\n");
    
    // Test case 1
    printf("Test 1: Username = 'John'\\n");
    char* result1 = hello("John");
    printf("%s\\n\\n", result1);
    free(result1);
    
    // Test case 2
    printf("Test 2: Username = 'Alice'\\n");
    char* result2 = hello("Alice");
    printf("%s\\n\\n", result2);
    free(result2);
    
    // Test case 3
    printf("Test 3: Username = 'Bob123'\\n");
    char* result3 = hello("Bob123");
    printf("%s\\n\\n", result3);
    free(result3);
    
    // Test case 4
    printf("Test 4: Username = 'User Name'\\n");
    char* result4 = hello("User Name");
    printf("%s\\n\\n", result4);
    free(result4);
    
    // Test case 5 - XSS prevention test
    printf("Test 5: Username with HTML = '<script>alert(\\"XSS\\")</script>'\\n");
    char* result5 = hello("<script>alert(\\"XSS\\")</script>");
    printf("%s\\n\\n", result5);
    free(result5);
    
    return 0;
}
