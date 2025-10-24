#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Sanitizes a string by escaping HTML special characters.
 * This function allocates new memory for the result. The caller is responsible
 * for freeing this memory using free().
 * @param input The raw string.
 * @return A pointer to the newly allocated and sanitized string, or NULL on failure.
 */
char* escapeHtml(const char* input) {
    if (input == NULL) {
        char* empty = malloc(1);
        if(empty == NULL) return NULL;
        empty[0] = '\0';
        return empty;
    }

    size_t input_len = strlen(input);
    size_t result_len = input_len;

    // First pass: calculate the required length for the new string
    for (size_t i = 0; i < input_len; ++i) {
        switch (input[i]) {
            case '&': result_len += 4; break; // &amp;
            case '<': result_len += 3; break; // &lt;
            case '>': result_len += 3; break; // &gt;
            case '"': result_len += 5; break; // &quot;
            case '\'': result_len += 4; break; // &#39;
        }
    }

    char* result = (char*)malloc(result_len + 1);
    if (result == NULL) {
        return NULL;
    }

    // Second pass: build the new string
    char* ptr = result;
    for (size_t i = 0; i < input_len; ++i) {
        switch (input[i]) {
            case '&': strcpy(ptr, "&amp;"); ptr += 5; break;
            case '<': strcpy(ptr, "&lt;"); ptr += 4; break;
            case '>': strcpy(ptr, "&gt;"); ptr += 4; break;
            case '"': strcpy(ptr, "&quot;"); ptr += 6; break;
            case '\'': strcpy(ptr, "&#39;"); ptr += 5; break;
            default: *ptr++ = input[i];
        }
    }
    *ptr = '\0';

    return result;
}

/**
 * Sanitizes user input and embeds it into a simple HTML page structure.
 * This function allocates new memory for the result. The caller is responsible
 * for freeing this memory using free().
 * @param userInput The raw string provided by the user.
 * @return A pointer to the new HTML string, or NULL on failure.
 */
char* generateSafeHtml(const char* userInput) {
    char* sanitizedInput = escapeHtml(userInput);
    if (sanitizedInput == NULL) {
        return NULL;
    }

    const char* prefix = "<html><body><p>User input: ";
    const char* suffix = "</p></body></html>";
    
    size_t final_len = strlen(prefix) + strlen(sanitizedInput) + strlen(suffix) + 1;
    char* finalHtml = (char*)malloc(final_len);

    if (finalHtml == NULL) {
        free(sanitizedInput); // Clean up intermediate allocation
        return NULL;
    }

    strcpy(finalHtml, prefix);
    strcat(finalHtml, sanitizedInput);
    strcat(finalHtml, suffix);

    free(sanitizedInput); // Free the intermediate string
    return finalHtml;
}

int main() {
    const char* testCases[] = {
        "Hello, World!",
        "1 < 5 is true",
        "He said, \"It's a & b > c\"",
        "<script>alert('XSS attack!');</script>",
        "",
        NULL // Sentinel value to mark the end of the array
    };

    printf("--- C Test Cases ---\n");
    for (int i = 0; testCases[i] != NULL; ++i) {
        const char* testCase = testCases[i];
        printf("Input: %s\n", testCase);
        char* safeHtml = generateSafeHtml(testCase);
        if (safeHtml) {
            printf("Output: %s\n", safeHtml);
            free(safeHtml); // Free the dynamically allocated memory
        } else {
            fprintf(stderr, "Output: (Memory allocation failed)\n");
        }
        printf("\n");
    }

    return 0;
}