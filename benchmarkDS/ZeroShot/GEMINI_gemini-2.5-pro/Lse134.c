#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration of the handler function type
struct Route;
typedef char* (*HandlerFunc)(const char*);

typedef struct {
    const char* pattern;
    HandlerFunc handler;
} Route;

/**
 * Converts special characters in a string to HTML-safe sequences.
 * This is crucial to prevent Cross-Site Scripting (XSS) attacks.
 * The caller is responsible for freeing the returned memory.
 *
 * @param input The string to escape.
 * @return A new dynamically allocated string with HTML-escaped content, or NULL on failure.
 */
char* escape(const char* input) {
    if (!input) return NULL;

    size_t extra_chars = 0;
    for (size_t i = 0; input[i] != '\0'; ++i) {
        switch (input[i]) {
            case '<':
            case '>':
                extra_chars += 3; // &lt; or &gt;
                break;
            case '&':
                extra_chars += 4; // &amp;
                break;
            case '"':
            case '\'':
                extra_chars += 5; // &quot; or &#39;
                break;
        }
    }

    size_t new_len = strlen(input) + extra_chars;
    char* escaped = (char*)malloc(new_len + 1);
    if (!escaped) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }

    char* dest = escaped;
    for (size_t i = 0; input[i] != '\0'; ++i) {
        switch (input[i]) {
            case '<':  dest += sprintf(dest, "&lt;");   break;
            case '>':  dest += sprintf(dest, "&gt;");   break;
            case '&':  dest += sprintf(dest, "&amp;");   break;
            case '"':  dest += sprintf(dest, "&quot;");  break;
            case '\'': dest += sprintf(dest, "&#39;");  break;
            default:   *dest++ = input[i];           break;
        }
    }
    *dest = '\0';
    return escaped;
}


/**
 * Handler function for the /hello/<username> route.
 * The caller is responsible for freeing the returned memory.
 *
 * @param username The username extracted from the URL.
 * @return A new dynamically allocated greeting string with the escaped username.
 */
char* hello(const char* username) {
    char* safe_username = escape(username);
    if (!safe_username) return NULL;

    // Allocate memory for "Hello " + safe_username + null terminator
    size_t final_len = strlen("Hello ") + strlen(safe_username) + 1;
    char* result = (char*)malloc(final_len);

    if (result) {
        sprintf(result, "Hello %s", safe_username);
    } else {
        perror("Failed to allocate memory for result string");
    }

    free(safe_username); // We are done with the escaped string
    return result;
}

// Global array of routes to simulate a router
Route routes[] = {
    {"/hello/%s", hello},
    {NULL, NULL} // Sentinel to mark the end of the array
};

/**
 * Handles an incoming URL request by finding a matching route and executing its handler.
 *
 * @param url The URL to handle.
 */
void handleRequest(const char* url) {
    printf("Request URL: %s\n", url);
    char username_buffer[256]; // Buffer to extract the username
    char* response = NULL;
    int matched = 0;

    for (int i = 0; routes[i].pattern != NULL; ++i) {
        // Use sscanf to match the pattern and extract the variable part.
        // This is a simple simulation; a real router would use more robust parsing.
        if (sscanf(url, routes[i].pattern, username_buffer) == 1) {
            // Check that there are no extra path segments
            char check_path[512];
            sprintf(check_path, "/hello/%s", username_buffer);
            if (strcmp(url, check_path) == 0) {
                 response = routes[i].handler(username_buffer);
                 matched = 1;
                 break;
            }
        }
    }

    if (matched && response) {
        printf("Response: %s\n\n", response);
        free(response); // Free the memory returned by the handler
    } else {
        printf("Response: 404 Not Found\n\n");
    }
}


int main() {
    printf("--- C Test Cases ---\n");
    const char* testUrls[] = {
        "/hello/Alice",
        "/hello/Bob",
        "/hello/<script>alert('xss')</script>", // Security test case
        "/hello/John Doe", // sscanf will stop at space, let's see how our simple router handles it
        "/hello/another/path" // Should not match
    };
    
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);
    for (int i = 0; i < num_tests; ++i) {
        handleRequest(testUrls[i]);
    }

    return 0;
}