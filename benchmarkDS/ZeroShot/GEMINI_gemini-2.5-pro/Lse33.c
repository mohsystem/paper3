#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * Checks if the URL has the domain "example.com".
 * This function simulates the logic of a web server route.
 * A full web server in a single C file is impractical, so this
 * function demonstrates the core security logic.
 *
 * @param url The URL string to validate.
 */
void redirect_invalid_url(const char* url) {
    if (url == NULL) {
        printf("Error: URL is NULL.\n");
        return;
    }
    
    // Find the protocol separator "://"
    const char* protocol_sep = strstr(url, "://");
    if (protocol_sep == NULL) {
        printf("Error: Malformed URL (missing '://'). Provided: '%s'\n", url);
        return;
    }

    // The hostname starts right after "://"
    const char* host_start = protocol_sep + 3;

    // Find the end of the hostname (delimiters are '/', ':', '?', '#', or end of string)
    const char* host_end = strpbrk(host_start, "/:?#");
    
    size_t host_len;
    if (host_end == NULL) {
        // The hostname is the rest of the string
        host_len = strlen(host_start);
    } else {
        host_len = host_end - host_start;
    }
    
    if (host_len == 0) {
        printf("Error: Hostname is empty in URL. Provided: '%s'\n", url);
        return;
    }

    // Secure check: compare the extracted hostname with "example.com"
    const char* valid_domain = "example.com";
    if (host_len == strlen(valid_domain) && strncmp(host_start, valid_domain, host_len) == 0) {
        printf("Redirecting to: %s\n", url);
    } else {
        // To print the invalid hostname, we need to copy it to a new buffer
        char* actual_host = (char*)malloc(host_len + 1);
        if (actual_host) {
            strncpy(actual_host, host_start, host_len);
            actual_host[host_len] = '\0';
            printf("Error: Invalid domain. Provided: '%s'. Redirection is only allowed to example.com.\n", actual_host);
            free(actual_host);
        } else {
            printf("Error: Invalid domain and failed to allocate memory to show hostname.\n");
        }
    }
}

int main() {
    // 5 Test Cases
    const char* test_cases[] = {
        // 1. Valid URL
        "https://example.com/path/to/page",
        // 2. Invalid domain
        "http://www.google.com",
        // 3. Subdomain exploit attempt
        "https://example.com.malicious.net/phishing",
        // 4. Valid domain with no path
        "http://example.com",
        // 5. Malformed URL
        "just-a-string"
    };
    
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Test %d: Checking URL '%s'\n", i + 1, test_cases[i]);
        printf("  Result: ");
        redirect_invalid_url(test_cases[i]);
        printf("\n");
    }

    return 0;
}