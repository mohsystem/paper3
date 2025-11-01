#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Sanitizes a string by escaping characters that have special meaning in HTML.
 * This function dynamically allocates memory for the new string. The caller is
 * responsible for freeing this memory using free().
 *
 * @param input The string to be sanitized.
 * @return A pointer to the newly allocated sanitized string, or NULL on failure.
 */
char* escapeHtml(const char* input) {
    if (!input) {
        // Return an empty, but valid, string for NULL input
        char* empty_str = (char*)malloc(1);
        if (empty_str) {
            empty_str[0] = '\0';
        }
        return empty_str;
    }

    size_t new_len = 0;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '<': new_len += 4; break; // &lt;
            case '>': new_len += 4; break; // &gt;
            case '&': new_len += 5; break; // &amp;
            case '"': new_len += 6; break; // &quot;
            case '\'': new_len += 5; break; // &#39;
            default:  new_len++; break;
        }
    }

    char* escaped_str = (char*)malloc(new_len + 1);
    if (!escaped_str) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }

    char* p_out = escaped_str;
    for (size_t i = 0; input[i] != '\0'; i++) {
        switch (input[i]) {
            case '<':  p_out = stpcpy(p_out, "&lt;");   break;
            case '>':  p_out = stpcpy(p_out, "&gt;");   break;
            case '&':  p_out = stpcpy(p_out, "&amp;");  break;
            case '"':  p_out = stpcpy(p_out, "&quot;"); break;
            case '\'': p_out = stpcpy(p_out, "&#39;");  break;
            default:  *p_out++ = input[i]; break;
        }
    }
    *p_out = '\0';

    return escaped_str;
}


/**
 * @brief Simulates handling an HTTP request for /hello.
 * The caller of this function is responsible for freeing the returned string.
 *
 * @param username The username read from the request body.
 * @return A pointer to a dynamically allocated string with the HTML response, or NULL on failure.
 */
char* http_hello_page(const char* username) {
    char* sanitizedUsername = escapeHtml(username);
    if (!sanitizedUsername) {
        return NULL; // Propagate memory allocation failure
    }

    const char* part1 = "<html><body><h1>Hello, ";
    const char* part3 = "!</h1></body></html>";
    
    // Calculate required size for the final buffer
    size_t len1 = strlen(part1);
    size_t len2 = strlen(sanitizedUsername);
    size_t len3 = strlen(part3);
    size_t total_len = len1 + len2 + len3;

    char* outputbuf = (char*)malloc(total_len + 1); // +1 for null terminator
    if (!outputbuf) {
        perror("Failed to allocate memory for output buffer");
        free(sanitizedUsername); // Clean up intermediate allocation
        return NULL;
    }
    
    // Securely construct the final string
    // snprintf is safer but requires more work. memcpy is efficient here.
    memcpy(outputbuf, part1, len1);
    memcpy(outputbuf + len1, sanitizedUsername, len2);
    memcpy(outputbuf + len1 + len2, part3, len3);
    outputbuf[total_len] = '\0';

    // Free the intermediate sanitized string as it's now copied into outputbuf
    free(sanitizedUsername);

    return outputbuf;
}

int main() {
    const char* testCases[] = {
        "Alice",
        "Bob Smith",
        "",
        "<script>alert('XSS Attack!')</script>",
        "O'Malley & Sons"
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < numTestCases; i++) {
        printf("Test Case %d:\n", i + 1);
        printf("Input:  %s\n", testCases[i]);
        char* response = http_hello_page(testCases[i]);
        if (response) {
            printf("Output: %s\n\n", response);
            // CRITICAL: Free the memory allocated by http_hello_page
            free(response);
        } else {
            printf("Output: FAILED (Memory allocation error)\n\n");
        }
    }

    return 0;
}