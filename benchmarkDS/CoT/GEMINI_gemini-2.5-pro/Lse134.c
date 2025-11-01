#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Converts special HTML characters to their corresponding entities.
 *
 * Dynamically allocates memory for the new escaped string. The caller is
 * responsible for freeing this memory. This function is essential to prevent
 * Cross-Site Scripting (XSS) vulnerabilities.
 *
 * @param input The raw string to escape.
 * @return A new, null-terminated string with HTML characters escaped,
 *         or NULL if memory allocation fails.
 */
char* escape(const char* input) {
    if (!input) return NULL;

    size_t input_len = strlen(input);
    size_t escaped_len = 0;

    // First pass: calculate the required length for the new string
    for (size_t i = 0; i < input_len; ++i) {
        switch (input[i]) {
            case '&':  escaped_len += 5; break; // &amp;
            case '<':  escaped_len += 4; break; // &lt;
            case '>':  escaped_len += 4; break; // &gt;
            case '"':  escaped_len += 6; break; // &quot;
            case '\'': escaped_len += 5; break; // &#39;
            default:   escaped_len += 1; break;
        }
    }

    char* escaped_str = (char*)malloc(escaped_len + 1);
    if (!escaped_str) {
        perror("Failed to allocate memory for escaped string");
        return NULL;
    }

    // Second pass: build the new string
    char* p = escaped_str;
    for (size_t i = 0; i < input_len; ++i) {
        switch (input[i]) {
            case '&':  memcpy(p, "&amp;", 5);  p += 5; break;
            case '<':  memcpy(p, "&lt;", 4);   p += 4; break;
            case '>':  memcpy(p, "&gt;", 4);   p += 4; break;
            case '"':  memcpy(p, "&quot;", 6); p += 6; break;
            case '\'': memcpy(p, "&#39;", 5);  p += 5; break;
            default:   *p++ = input[i]; break;
        }
    }
    *p = '\0'; // Null-terminate the string

    return escaped_str;
}

/**
 * @brief The handler function for the /hello/<username> route.
 *
 * @param username The username extracted from the URL.
 * @return A new greeting string with the username safely escaped. The caller
 *         is responsible for freeing the returned string.
 */
char* hello(const char* username) {
    char* escaped_username = escape(username);
    if (!escaped_username) return NULL;

    const char* prefix = "Hello ";
    size_t prefix_len = strlen(prefix);
    size_t escaped_len = strlen(escaped_username);
    
    char* result = (char*)malloc(prefix_len + escaped_len + 1);
    if (!result) {
        perror("Failed to allocate memory for result string");
        free(escaped_username); // Clean up intermediate allocation
        return NULL;
    }
    
    memcpy(result, prefix, prefix_len);
    memcpy(result + prefix_len, escaped_username, escaped_len);
    result[prefix_len + escaped_len] = '\0';

    free(escaped_username); // Free the intermediate escaped string
    return result;
}

/**
 * @brief Simulates routing a request to the correct handler.
 *
 * This function checks if the URL matches the pattern "/hello/<username>"
 * and, if so, calls the hello() handler.
 *
 * @param url The request URL.
 * @return A new response string that must be freed by the caller,
 *         or a static "404 Not Found" string.
 */
char* handle_request(const char* url) {
    const char* prefix = "/hello/";
    if (strncmp(url, prefix, strlen(prefix)) == 0) {
        const char* username = url + strlen(prefix);
        return hello(username);
    }
    // Return a dynamically allocated string for consistency, though not strictly necessary
    char* not_found_msg = (char*)malloc(14);
    if(not_found_msg) strcpy(not_found_msg, "404 Not Found");
    return not_found_msg;
}

int main() {
    printf("C Test Cases:\n");
    const char* test_urls[] = {
        "/hello/World",
        "/hello/Jane Doe",
        "/hello/<script>alert('XSS')</script>",
        "/hello/user&'\"<>",
        "/hello/"
    };
    int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Request: %s\n", test_urls[i]);
        char* response = handle_request(test_urls[i]);
        if (response) {
            printf("Response: %s\n", response);
            free(response); // IMPORTANT: Free the memory allocated by the handler
        } else {
            printf("Response: (Error creating response)\n");
        }
        printf("---\n");
    }

    return 0;
}