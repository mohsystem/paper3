#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Escapes special HTML characters in a string to prevent XSS.
 * NOTE: The caller is responsible for freeing the returned memory.
 *
 * @param input The raw string to be escaped.
 * @return A new dynamically allocated string with HTML-escaped content, or NULL on error.
 */
char* escapeHtml(const char* input) {
    if (!input) {
        return NULL;
    }

    // First pass: calculate the required length for the new string
    size_t new_len = 0;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&':  new_len += 5; break; // &amp;
            case '<':  new_len += 4; break; // &lt;
            case '>':  new_len += 4; break; // &gt;
            case '"':  new_len += 6; break; // &quot;
            case '\'': new_len += 5; break; // &#39;
            default:   new_len += 1; break;
        }
    }

    // Allocate memory for the new string (+1 for null terminator)
    char* output = (char*)malloc(new_len + 1);
    if (!output) {
        return NULL; // Memory allocation failed
    }

    // Second pass: build the new string
    char* ptr = output;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '&':  ptr = stpcpy(ptr, "&amp;"); break;
            case '<':  ptr = stpcpy(ptr, "&lt;"); break;
            case '>':  ptr = stpcpy(ptr, "&gt;"); break;
            case '"':  ptr = stpcpy(ptr, "&quot;"); break;
            case '\'': ptr = stpcpy(ptr, "&#39;"); break;
            default:   *ptr++ = input[i]; break;
        }
    }
    *ptr = '\0'; // Null-terminate the string

    return output;
}

/**
 * Generates a simple HTML page string displaying user-supplied input securely.
 * NOTE: The caller is responsible for freeing the returned memory.
 *
 * @param userInput The user-supplied string.
 * @return A new dynamically allocated string containing the HTML page, or NULL on error.
 */
char* generateWebPage(const char* userInput) {
    char* escapedInput = escapeHtml(userInput);
    if (!escapedInput) {
        return NULL;
    }

    const char* template1 = "<!DOCTYPE html>\n<html>\n<head>\n  <title>User Input</title>\n</head>\n<body>\n  <h1>User Comment:</h1>\n  <p>";
    const char* template2 = "</p>\n</body>\n</html>";
    
    size_t total_len = strlen(template1) + strlen(escapedInput) + strlen(template2) + 1;
    char* webPage = (char*)malloc(total_len);

    if (webPage) {
        strcpy(webPage, template1);
        strcat(webPage, escapedInput);
        strcat(webPage, template2);
    }
    
    free(escapedInput); // Free the memory from escapeHtml
    return webPage;
}

int main() {
    const char* testCases[] = {
        "Hello, World!",
        "1 < 5",
        "This is a 'quote' & an \"ampersand\"",
        "<script>alert('XSS Attack!')</script>",
        ""
    };

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < 5; i++) {
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Input: %s\n", testCases[i]);
        char* webPage = generateWebPage(testCases[i]);
        if (webPage) {
            printf("Generated HTML:\n%s\n", webPage);
            free(webPage); // Free the final generated page
        } else {
            printf("Error generating web page.\n");
        }
    }

    return 0;
}