#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * Checks if the URL's domain is 'example.com' and returns an appropriate message.
 * This is a simplified, non-production-ready URL parser for demonstration.
 * In a real-world C application, a robust library like 'libcurl' or 'uriparser'
 * should be used to prevent security vulnerabilities from parsing edge cases.
 * This implementation is vulnerable to complex edge cases and should not be used in production.
 *
 * @param urlString The URL to check.
 * @param outputBuffer A buffer to store the output message.
 * @param bufferSize The size of the output buffer.
 */
void redirect_invalid_url(const char* urlString, char* outputBuffer, size_t bufferSize) {
    if (urlString == NULL || *urlString == '\0') {
        snprintf(outputBuffer, bufferSize, "Error: URL is empty.");
        return;
    }

    // Find the protocol separator "://"
    const char* authority_start = strstr(urlString, "://");
    if (authority_start == NULL) {
        snprintf(outputBuffer, bufferSize, "Error: Malformed URL provided (missing '://').");
        return;
    }
    authority_start += 3; // Move past "://"

    // Find the end of the authority part (marked by '/', '?', '#', or end of string)
    const char* authority_end = strpbrk(authority_start, "/?#");
    if (authority_end == NULL) {
        authority_end = urlString + strlen(urlString);
    }
    
    // To prevent open redirects, find the last '@' and use the part after it as the host.
    const char* host_start = authority_start;
    const char* user_info_marker = strchr(authority_start, '@');
    // Ensure we are not looking past the authority part
    if (user_info_marker != NULL && user_info_marker < authority_end) {
        host_start = user_info_marker + 1;
    }

    // Find port separator ':' within the host part
    const char* port_marker = strchr(host_start, ':');
    const char* host_end = authority_end;
    if (port_marker != NULL && port_marker < authority_end) {
        host_end = port_marker;
    }

    size_t host_len = host_end - host_start;
    if (host_len == 0) {
        snprintf(outputBuffer, bufferSize, "Error: Could not determine host from URL.");
        return;
    }

    // Copy host to a temporary buffer for manipulation and comparison
    // We add 1 for the null terminator.
    if (host_len + 1 > 256) { // 255 is max length for a DNS name
        snprintf(outputBuffer, bufferSize, "Error: Host name is too long.");
        return;
    }
    char host[256];
    strncpy(host, host_start, host_len);
    host[host_len] = '\0';

    // Convert host to lower case for case-insensitive comparison
    for (int i = 0; host[i]; i++) {
        host[i] = tolower(host[i]);
    }

    // Perform a strict comparison against the allowed domain.
    if (strcmp(host, "example.com") == 0) {
        snprintf(outputBuffer, bufferSize, "Redirecting to %s", urlString);
    } else {
        snprintf(outputBuffer, bufferSize, "Error: Domain '%s' is not allowed.", host);
    }
}

int main() {
    const char* testUrls[] = {
        // 1. Valid URL
        "https://example.com/some/path",
        // 2. Another valid URL with a different scheme
        "http://example.com",
        // 3. Invalid domain
        "https://www.google.com",
        // 4. Open redirect attempt via query parameter (should be blocked)
        "https://evil.com?q=example.com",
        // 5. Open redirect attempt via user info (should be blocked)
        "https://example.com@evil.com/login"
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);
    char result[512];

    printf("Running C Test Cases:\n");
    for (int i = 0; i < num_tests; i++) {
        printf("Input: %s\n", testUrls[i]);
        redirect_invalid_url(testUrls[i], result, sizeof(result));
        printf("Output: %s\n", result);
        printf("--------------------\n");
    }

    return 0;
}