#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
char* escape(const char* input);
char* hello(const char* username);
char* dispatch(const char* url);

/**
 * A simple HTML escape function to prevent XSS.
 * This function allocates new memory for the result, which must be freed by the caller.
 * Rule #3: Ensure that all buffer operations strictly adhere to the buffer's boundaries.
 * Rule #5: Check all allocation results.
 */
char* escape(const char* input) {
    if (input == NULL) {
        char* empty_str = (char*)malloc(1);
        if (empty_str) empty_str[0] = '\0';
        return empty_str;
    }

    size_t new_len = 0;
    for (size_t i = 0; input[i] != '\0'; ++i) {
        switch (input[i]) {
            case '&': new_len += 5; break; // &amp;
            case '\"': new_len += 6; break; // &quot;
            case '\'': new_len += 6; break; // &#x27;
            case '<': new_len += 4; break; // &lt;
            case '>': new_len += 4; break; // &gt;
            default: new_len += 1; break;
        }
    }

    char* output = (char*)malloc(new_len + 1);
    if (output == NULL) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }

    char* dest = output;
    for (size_t i = 0; input[i] != '\0'; ++i) {
        switch (input[i]) {
            case '&':  dest = stpcpy(dest, "&amp;"); break;
            case '\"': dest = stpcpy(dest, "&quot;"); break;
            case '\'': dest = stpcpy(dest, "&#x27;"); break;
            case '<':  dest = stpcpy(dest, "&lt;"); break;
            case '>':  dest = stpcpy(dest, "&gt;"); break;
            default:   *dest++ = input[i]; break;
        }
    }
    *dest = '\0';

    return output;
}

/**
 * Handler function for the /hello/<username> route.
 * This function allocates new memory for the result, which must be freed by the caller.
 */
char* hello(const char* username) {
    // Rule #1: Sanitize input before processing.
    char* escaped_username = escape(username);
    if (escaped_username == NULL) {
        return NULL; // Propagate memory allocation failure
    }

    const char* prefix = "Hello, ";
    const char* suffix = "!";
    // Use snprintf to determine the required buffer size to prevent overflows.
    size_t size = snprintf(NULL, 0, "%s%s%s", prefix, escaped_username, suffix) + 1;
    
    char* result = (char*)malloc(size);
    if (result == NULL) {
        perror("Failed to allocate memory for hello string");
        free(escaped_username); // Clean up allocated memory on failure
        return NULL;
    }

    snprintf(result, size, "%s%s%s", prefix, escaped_username, suffix);
    free(escaped_username); // Free the intermediate escaped string

    return result;
}

/**
 * Dispatches a request based on the URL.
 * This is a simple simulation of a router.
 * It allocates memory for the result, which must be freed.
 */
char* dispatch(const char* url) {
    const char* prefix = "/hello/";
    if (strncmp(url, prefix, strlen(prefix)) == 0) {
        const char* username = url + strlen(prefix);
        if (strlen(username) > 0) {
            return hello(username);
        }
    }

    // For "404 Not Found", also allocate so the caller can always free.
    char* not_found_msg = (char*)malloc(16);
    if (not_found_msg == NULL) {
        perror("Failed to allocate memory for 404 message");
        return NULL;
    }
    strcpy(not_found_msg, "404 Not Found");
    return not_found_msg;
}


int main() {
    const char* test_urls[] = {
        "/hello/World",
        "/hello/C",
        "/hello/<script>alert('xss')</script>",
        "/hello/O'Malley",
        "/hello/a&b"
    };
    
    printf("--- C Test Cases ---\n");
    for (int i = 0; i < 5; ++i) {
        printf("Request URL: %s\n", test_urls[i]);
        char* response = dispatch(test_urls[i]);
        if (response != NULL) {
            printf("Response: %s\n\n", response);
            free(response); // Rule #5: All allocated memory freed exactly once
        } else {
            printf("Response: (Error)\n\n");
        }
    }
    
    return 0;
}