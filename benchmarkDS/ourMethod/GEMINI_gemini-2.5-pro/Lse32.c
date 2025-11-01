#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Checks if a URL is from example.com and simulates a server response.
 *
 * This function implements the core validation logic. In a real web application,
 * it would be part of a request handler that generates an HTTP response.
 * Here, it prints the simulated response to the console.
 *
 * @param url The URL to validate.
 */
void redirect_invalid_url(const char* url) {
    if (url == NULL) {
        printf("Status: 400 Bad Request\n");
        printf("Error: URL is NULL.\n\n");
        // In a web server, this would be a 400 Bad Request response.
        return;
    }
    
    // Rule #1: Input validation based on a strict whitelist of URL prefixes.
    // This is much safer than trying to parse the URL manually.
    const char* valid_http_prefix = "http://example.com/";
    const char* valid_https_prefix = "https://example.com/";
    const char* valid_http_exact = "http://example.com";
    const char* valid_https_exact = "https://example.com";

    int is_valid = 0;
    if ((strcmp(url, valid_http_exact) == 0) ||
        (strcmp(url, valid_https_exact) == 0) ||
        (strncmp(url, valid_http_prefix, strlen(valid_http_prefix)) == 0) ||
        (strncmp(url, valid_https_prefix, strlen(valid_https_prefix)) == 0)) {
        is_valid = 1;
    }

    if (is_valid) {
        printf("Status: 302 Found\n");
        printf("Location: %s\n\n", url);
        // This simulates a redirect response.
    } else {
        printf("Status: 400 Bad Request\n");
        printf("Error: Redirects are only allowed to example.com.\n\n");
        // This simulates an error response.
    }
}

int main() {
    printf("C does not have a standard built-in web server.\n");
    printf("This program demonstrates the required URL validation logic for the redirect.\n");
    printf("It simulates the server's response to different URLs.\n\n");

    // 5 Test Cases
    const char* test_urls[] = {
        // Test case 1: Valid HTTP URL
        "http://example.com",
        // Test case 2: Valid HTTPS URL with a path
        "https://example.com/some/path",
        // Test case 3: Invalid domain
        "http://google.com",
        // Test case 4: Invalid domain that looks similar
        "http://example.com.malicious.com",
        // Test case 5: Invalid scheme
        "ftp://example.com",
        NULL // End of test cases
    };

    for (int i = 0; test_urls[i] != NULL; ++i) {
        printf("--- Testing URL: \"%s\" ---\n", test_urls[i]);
        redirect_invalid_url(test_urls[i]);
    }
    
    return 0;
}