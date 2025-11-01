#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * Checks if a URL's host is 'example.com' and returns a redirect message.
 * Otherwise, returns a 404 error message. This implementation is secure
 * against open redirect vulnerabilities by strictly checking the hostname.
 *
 * Note: C standard library does not have a built-in URL parser. This is a
 * simplified manual implementation. For production code, a robust third-party
 * library is highly recommended.
 *
 * @param url The URL string to validate.
 * @return A constant string literal indicating redirection or a 404 error.
 */
const char* redirect_url(const char* url) {
    if (url == NULL) {
        return "404 Not Found";
    }

    const char* protocol_separator = "://";
    const char* host_start = strstr(url, protocol_separator);

    if (host_start == NULL) {
        return "404 Not Found";
    }
    
    host_start += strlen(protocol_separator);

    // Find the end of the host part (marked by ':', '/', '?', or end of string)
    const char* host_end = strpbrk(host_start, ":/?");
    
    size_t host_len;
    if (host_end == NULL) {
        host_len = strlen(host_start);
    } else {
        host_len = host_end - host_start;
    }

    if (host_len == 0) {
        return "404 Not Found";
    }

    // Perform a case-insensitive comparison
    const char* target_host = "example.com";
    if (strlen(target_host) != host_len) {
        return "404 Not Found";
    }

    for (size_t i = 0; i < host_len; ++i) {
        if (tolower((unsigned char)host_start[i]) != (unsigned char)target_host[i]) {
            return "404 Not Found";
        }
    }
    
    // NOTE: Returning a static string. A dynamic string with the original URL
    // would require manual memory management (malloc/free).
    return "Redirecting to a valid example.com URL";
}

int main() {
    const char* testUrls[] = {
        "https://example.com/path/to/resource",
        "http://www.google.com",
        "https://evil-example.com",
        "https://example.com.hacker.com/login",
        "ftp://example.com:21/files"
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Input: %s\n", testUrls[i]);
        const char* result = redirect_url(testUrls[i]);
        // For clarity, if the result is a success, we print the full message
        // here, since the function itself returns a generic success string.
        if (strcmp(result, "Redirecting to a valid example.com URL") == 0) {
            printf("Output: Redirecting to: %s\n", testUrls[i]);
        } else {
            printf("Output: %s\n", result);
        }
        printf("--------------------\n");
    }

    return 0;
}