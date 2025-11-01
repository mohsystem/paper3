#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Standard C does not include a web server or a regular expression library.
// This code demonstrates the core validation logic using secure string parsing,
// which is more portable than relying on non-standard or platform-specific
// libraries like POSIX regex.h. In a real application, this function would be
// part of a larger web application using a library like libmicrohttpd.

/**
 * @brief Validates a target URL and returns the redirect destination.
 * 
 * @param target The target URL to validate.
 * @return A new heap-allocated string containing the redirect URL.
 *         The caller is responsible for freeing this memory.
 *         Returns "/" for invalid targets or target.
 */
char* get_redirect_url(const char* target) {
    const char* default_redirect = "/";
    char* result_url = NULL;

    int is_valid = 0;

    if (target != NULL) {
        // 1. Check for valid scheme (http:// or https://)
        if (strncmp(target, "http://", 7) == 0 || strncmp(target, "https://", 8) == 0) {
            // 2. Find the start of the host
            const char* host_start = strstr(target, "://") + 3;

            // 3. Find the end of the host (ends at /, ?, #, or end of string)
            const char* host_end = strpbrk(host_start, "/?#");
            size_t host_len;
            if (host_end == NULL) {
                host_len = strlen(host_start);
            } else {
                host_len = host_end - host_start;
            }

            // 4. Copy host to a temporary buffer for validation
            if (host_len > 0) {
                char* host = (char*)malloc(host_len + 1);
                if (host != NULL) {
                    strncpy(host, host_start, host_len);
                    host[host_len] = '\0';

                    // 5. Prevent user-info bypass (e.g., http://user@host)
                    if (strchr(host, '@') == NULL) {
                        // 6. Remove port number if it exists
                        char* port_colon = strchr(host, ':');
                        if (port_colon != NULL) {
                            *port_colon = '\0';
                        }
                        
                        // 7. Validate the hostname
                        const char* suffix = ".example.com";
                        size_t host_len_no_port = strlen(host);
                        size_t suffix_len = strlen(suffix);
                        if (strcmp(host, "example.com") == 0) {
                            is_valid = 1;
                        } else if (host_len_no_port > suffix_len && 
                                   strcmp(host + host_len_no_port - suffix_len, suffix) == 0) {
                            is_valid = 1;
                        }
                    }
                    free(host);
                }
            }
        }
    }

    if (is_valid) {
        result_url = (char*)malloc(strlen(target) + 1);
        if (result_url != NULL) {
            strcpy(result_url, target);
        }
    }
    
    // If anything failed or was invalid, fall back to the default
    if (result_url == NULL) {
        result_url = (char*)malloc(strlen(default_redirect) + 1);
        if (result_url != NULL) {
            strcpy(result_url, default_redirect);
        } else {
            // Catastrophic failure, should not happen.
            exit(1);
        }
    }

    return result_url;
}

void run_test_case(const char* input, const char* expected) {
    char* result = get_redirect_url(input);
    printf("Input:    %s\n", input);
    printf("Output:   %s\n", result);
    printf("Expected: %s\n", expected);
    if (strcmp(result, expected) == 0) {
        printf("Result:   PASS\n");
    } else {
        printf("Result:   FAIL\n");
    }
    printf("------------------------------------\n");
    free(result);
}

int main() {
    // Main class name as per instructions (conceptual in C)
    typedef struct Lse29 {} Lse29;

    printf("Running C test cases...\n");
    run_test_case("https://sub.example.com/path/to/resource", "https://sub.example.com/path/to/resource");
    run_test_case("http://example.com", "http://example.com");
    run_test_case("https://www.google.com", "/");
    run_test_case("https://example.com.malicious.com", "/");
    run_test_case("javascript:alert(1)//http://example.com", "/");
    run_test_case("https://user@evil.com?q=example.com", "/");
    run_test_case("https://example.com:8443/test", "https://example.com:8443/test");
    return 0;
}