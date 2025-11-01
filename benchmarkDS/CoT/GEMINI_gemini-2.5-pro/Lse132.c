#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// The content of the "hello.html" template file.
const char* HELLO_TEMPLATE =
    "<!DOCTYPE html>\n"
    "<html>\n"
    "<head>\n"
    "    <title>Hello Page</title>\n"
    "</head>\n"
    "<body>\n"
    "    <h1>Hello, %s!</h1>\n"
    "</body>\n"
    "</html>";

/**
 * Escapes special HTML characters to prevent Cross-Site Scripting (XSS).
 * The caller is responsible for freeing the returned string.
 * @param data The input string to escape.
 * @return A new dynamically allocated string that is HTML-safe, or NULL on error.
 */
char* htmlEscape(const char* data) {
    if (data == NULL) return NULL;

    size_t data_len = strlen(data);
    size_t escaped_len = 0;
    
    // First pass: calculate the required length for the escaped string
    for (size_t i = 0; i < data_len; ++i) {
        switch (data[i]) {
            case '&':  escaped_len += 5; break; // &amp;
            case '\"': escaped_len += 6; break; // &quot;
            case '\'': escaped_len += 5; break; // &#39;
            case '<':  escaped_len += 4; break; // &lt;
            case '>':  escaped_len += 4; break; // &gt;
            default:   escaped_len++;
        }
    }

    char* escaped_str = (char*)malloc(escaped_len + 1);
    if (escaped_str == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Second pass: build the escaped string
    char* p = escaped_str;
    for (size_t i = 0; i < data_len; ++i) {
        switch (data[i]) {
            case '&':  p += sprintf(p, "&amp;");   break;
            case '\"': p += sprintf(p, "&quot;");  break;
            case '\'': p += sprintf(p, "&#39;");  break;
            case '<':  p += sprintf(p, "&lt;");    break;
            case '>':  p += sprintf(p, "&gt;");    break;
            default:   *p++ = data[i];
        }
    }
    *p = '\0';
    
    return escaped_str;
}

/**
 * Renders the hello page. The caller must free the returned string.
 * @param username The username to be displayed.
 * @return A dynamically allocated string containing the full HTML page.
 */
char* renderHelloPage(const char* username) {
    char* safe_username = htmlEscape(username);
    if (safe_username == NULL) return NULL;

    // Calculate buffer size needed for the final HTML string
    size_t buffer_size = strlen(HELLO_TEMPLATE) + strlen(safe_username) + 1;
    char* response = (char*)malloc(buffer_size);
    if (response == NULL) {
        perror("Failed to allocate memory for response");
        free(safe_username); // Clean up
        return NULL;
    }

    snprintf(response, buffer_size, HELLO_TEMPLATE, safe_username);
    
    free(safe_username); // Free the intermediate escaped string
    return response;
}

/**
 * Simulates a URL router. The caller must free the returned string.
 * @param path The URL path from the request.
 * @return The HTTP response body as a string, or an error string.
 */
char* routeRequest(const char* path) {
    const char* prefix = "/hello/";
    if (strncmp(path, prefix, strlen(prefix)) == 0) {
        const char* username = path + strlen(prefix);
        if (strlen(username) > 0) {
            return renderHelloPage(username);
        }
    }
    
    // For non-matching routes, return a static string.
    // To be consistent, we allocate a new string.
    char* not_found_msg = strdup("404 Not Found");
    if (not_found_msg == NULL) {
        perror("Failed to allocate memory for 404 message");
    }
    return not_found_msg;
}

int main() {
    printf("--- Running 5 Test Cases ---\n");
    
    const char* test_paths[] = {
        // Test Case 1: Simple username
        "/hello/World",
        // Test Case 2: Username with spaces and symbols
        "/hello/Bob & Alice",
        // Test Case 3: Numeric username
        "/hello/78910",
        // Test Case 4: Non-matching URL
        "/index.html",
        // Test Case 5: Malicious XSS attempt (should be escaped)
        "/hello/<script>window.location='http://evil.com'</script>"
    };
    int num_tests = sizeof(test_paths) / sizeof(test_paths[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Test Case %d (Input Path: %s):\n", i + 1, test_paths[i]);
        char* response = routeRequest(test_paths[i]);
        if (response) {
            printf("%s\n", response);
            free(response); // Free the memory returned by the handler
        } else {
            printf("Error: No response generated.\n");
        }
        printf("--------------------\n");
    }

    return 0;
}