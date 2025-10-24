#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Securely escapes a user input string to prevent XSS attacks.
 * Note: The caller is responsible for freeing the returned string.
 *
 * @param userInput The raw string from the user.
 * @return A dynamically allocated string with sanitized content, or NULL on error.
 */
char* sanitizeForHtml(const char* userInput) {
    if (userInput == NULL) {
        char* emptyStr = (char*)malloc(1);
        if (emptyStr) emptyStr[0] = '\0';
        return emptyStr;
    }

    // First pass: calculate the required length for the sanitized string
    size_t required_len = 0;
    for (size_t i = 0; userInput[i] != '\0'; i++) {
        switch (userInput[i]) {
            case '<': required_len += 4; break; // &lt;
            case '>': required_len += 4; break; // &gt;
            case '&': required_len += 5; break; // &amp;
            case '"': required_len += 6; break; // &quot;
            case '\'': required_len += 5; break; // &#39;
            default: required_len++; break;
        }
    }

    char* sanitized = (char*)malloc(required_len + 1);
    if (sanitized == NULL) {
        return NULL; // Memory allocation failed
    }

    // Second pass: build the sanitized string
    size_t j = 0;
    for (size_t i = 0; userInput[i] != '\0'; i++) {
        switch (userInput[i]) {
            case '<': j += sprintf(sanitized + j, "&lt;"); break;
            case '>': j += sprintf(sanitized + j, "&gt;"); break;
            case '&': j += sprintf(sanitized + j, "&amp;"); break;
            case '"': j += sprintf(sanitized + j, "&quot;"); break;
            case '\'': j += sprintf(sanitized + j, "&#39;"); break;
            default: sanitized[j++] = userInput[i]; break;
        }
    }
    sanitized[j] = '\0';
    return sanitized;
}

/**
 * Wraps the sanitized user input in a basic HTML page.
 * Note: The caller is responsible for freeing the returned string.
 *
 * @param userInput The raw string from the user.
 * @return A dynamically allocated string containing a complete HTML page, or NULL on error.
 */
char* secureDisplay(const char* userInput) {
    char* sanitizedInput = sanitizeForHtml(userInput);
    if (sanitizedInput == NULL) return NULL;

    const char* prefix = "<html><body><h1>User Input:</h1><p>";
    const char* suffix = "</p></body></html>";
    
    size_t total_len = strlen(prefix) + strlen(sanitizedInput) + strlen(suffix);
    char* finalHtml = (char*)malloc(total_len + 1);

    if (finalHtml == NULL) {
        free(sanitizedInput);
        return NULL;
    }
    
    strcpy(finalHtml, prefix);
    strcat(finalHtml, sanitizedInput);
    strcat(finalHtml, suffix);
    
    free(sanitizedInput);
    
    return finalHtml;
}

int main() {
    // 5 Test Cases
    const char* testCases[] = {
        "Hello, World!", // Normal input
        "<script>alert('XSS');</script>", // Malicious script
        "John & Doe > Company", // Input with special HTML characters
        "This is a \"quote\".", // Input with quotes
        "1 < 2 && 3 > 1" // Another input with special characters
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- Running C Test Cases ---\n");
    for (int i = 0; i < numTestCases; i++) {
        printf("\nTest Case %d:\n", i + 1);
        printf("Original Input: %s\n", testCases[i]);
        char* safeHtml = secureDisplay(testCases[i]);
        if (safeHtml) {
            printf("Generated Safe HTML: %s\n", safeHtml);
            free(safeHtml);
        } else {
            printf("Memory allocation failed.\n");
        }
    }

    return 0;
}