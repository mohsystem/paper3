#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Checks the domain of a URL and returns a redirect or 404 message.
 *
 * This function uses simple string manipulation. It may not handle all
 * edge cases of complex URLs.
 *
 * @param url The URL string to check.
 * @return A dynamically allocated string with the result. The caller is
 *         responsible for freeing this memory using free(). Returns NULL on
 *         memory allocation failure.
 */
char* redirect_url(const char* url) {
    if (url == NULL || *url == '\0') {
        char* result = (char*)malloc(sizeof("404 Not Found"));
        if (result == NULL) return NULL;
        strcpy(result, "404 Not Found");
        return result;
    }

    const char* scheme_pos = strstr(url, "://");
    if (scheme_pos == NULL) {
        char* result = (char*)malloc(sizeof("404 Not Found"));
        if (result == NULL) return NULL;
        strcpy(result, "404 Not Found");
        return result;
    }

    const char* domain_start = scheme_pos + 3;

    // Find the length of the domain part by searching for the first of several delimiters
    size_t domain_len = strcspn(domain_start, "/:?");

    const char* target_domain = "example.com";
    size_t target_len = strlen(target_domain);

    char* result = NULL;
    if (domain_len == target_len && strncmp(domain_start, target_domain, target_len) == 0) {
        // The domains match. Create the redirect string.
        // Use snprintf to determine the required buffer size for safety.
        size_t needed = (size_t)snprintf(NULL, 0, "Redirecting to %s", url) + 1;
        result = (char*)malloc(needed);
        if (result == NULL) {
            return NULL;
        }
        snprintf(result, needed, "Redirecting to %s", url);
    } else {
        // No match. Create the 404 string.
        result = (char*)malloc(sizeof("404 Not Found"));
        if (result == NULL) return NULL;
        strcpy(result, "404 Not Found");
    }

    return result;
}

int main() {
    const char* test_cases[] = {
        "https://example.com/some/path",  // Case 1: Match
        "http://google.com/search",       // Case 2: No Match
        "invalid-url-format",             // Case 3: Invalid format
        "https://another-site.net",       // Case 4: Another No Match
        "http://example.com"              // Case 5: Match without path
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    printf("--- Running 5 Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Input: %s\n", test_cases[i]);
        char* output = redirect_url(test_cases[i]);
        if (output != NULL) {
            printf("Output: %s\n", output);
            free(output); // IMPORTANT: Free the allocated memory
        } else {
            fprintf(stderr, "Output: Memory allocation failed!\n");
        }
        printf("--------------------\n");
    }

    return 0;
}