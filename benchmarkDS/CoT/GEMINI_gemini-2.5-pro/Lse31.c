#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * Checks if a URL's domain is 'example.com' and returns a corresponding status message.
 * Note: This implementation uses a static buffer for the return value to meet the
 * prompt's requirements without dynamic memory allocation. This is not thread-safe.
 * @param url The URL to check.
 * @return A string indicating redirection or a 404 error.
 */
const char* redirect_url(const char* url) {
    // Static buffer for constructing the redirect message.
    static char redirect_buffer[2048];
    // Static buffer for holding the extracted domain, for case-insensitive comparison.
    static char domain_buffer[256];

    if (url == NULL) {
        return "404 Not Found";
    }

    const char* protocol_delimiter = "://";
    const char* domain_start = strstr(url, protocol_delimiter);

    if (domain_start == NULL) {
        return "404 Not Found";
    }

    domain_start += strlen(protocol_delimiter);

    const char* domain_end = strchr(domain_start, '/');
    
    size_t domain_len;
    if (domain_end == NULL) {
        domain_len = strlen(domain_start);
    } else {
        domain_len = domain_end - domain_start;
    }

    // Ensure domain fits in our buffer
    if (domain_len >= sizeof(domain_buffer)) {
        return "404 Not Found";
    }
    
    // Copy domain to a mutable buffer to convert to lowercase
    strncpy(domain_buffer, domain_start, domain_len);
    domain_buffer[domain_len] = '\0';
    for(int i = 0; domain_buffer[i]; i++){
        domain_buffer[i] = tolower(domain_buffer[i]);
    }
    
    if (strcmp(domain_buffer, "example.com") == 0) {
        snprintf(redirect_buffer, sizeof(redirect_buffer), "Redirecting to %s", url);
        return redirect_buffer;
    } else {
        return "404 Not Found";
    }
}

int main() {
    const char* test_urls[] = {
        "http://example.com/home",
        "https://EXAMPLE.com/login?user=1",
        "http://www.google.com",
        "https://sub.example.com",
        "not-a-valid-url"
    };
    int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);

    printf("--- Running 5 Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Input: %s\n", test_urls[i]);
        printf("Output: %s\n", redirect_url(test_urls[i]));
        printf("--------------------\n");
    }

    return 0;
}