#include <stdio.h>
#include <string.h>

/**
 * Checks if the URL's domain is "example.com".
 * NOTE: This function is not thread-safe due to the use of a static buffer for the return value.
 *
 * @param url The URL string to check.
 * @return A constant character pointer to a message.
 */
const char* redirectInvalidUrl(const char* url) {
    static char responseBuffer[1024]; // Static buffer for the response message
    char host[256];                   // Buffer to hold the extracted hostname

    const char* protocol_delimiter = "://";
    const char* protocol_pos = strstr(url, protocol_delimiter);

    if (protocol_pos == NULL) {
        snprintf(responseBuffer, sizeof(responseBuffer), "Error: Malformed URL provided (missing '://').");
        return responseBuffer;
    }

    const char* host_start = protocol_pos + strlen(protocol_delimiter);
    if (*host_start == '\0') {
         snprintf(responseBuffer, sizeof(responseBuffer), "Error: Malformed URL provided (empty host).");
        return responseBuffer;
    }
    
    const char* host_end = strchr(host_start, '/');
    
    size_t host_len;
    if (host_end == NULL) {
        // No path, host is the rest of the string
        host_len = strlen(host_start);
    } else {
        host_len = host_end - host_start;
    }

    if (host_len == 0) {
        snprintf(responseBuffer, sizeof(responseBuffer), "Error: Malformed URL provided (empty host).");
        return responseBuffer;
    }
    
    // Ensure host_len does not exceed our buffer size minus 1 for the null terminator
    if (host_len >= sizeof(host)) {
        snprintf(responseBuffer, sizeof(responseBuffer), "Error: Hostname is too long.");
        return responseBuffer;
    }

    strncpy(host, host_start, host_len);
    host[host_len] = '\0'; // Null-terminate the extracted host string

    if (strcmp(host, "example.com") == 0) {
        snprintf(responseBuffer, sizeof(responseBuffer), "Redirecting to %s", url);
    } else {
        snprintf(responseBuffer, sizeof(responseBuffer), "Error: Invalid domain. Only example.com is allowed. Domain was: %s", host);
    }

    return responseBuffer;
}

int main() {
    const char* testUrls[] = {
        "http://example.com/path/to/page",
        "https://www.google.com",
        "http://test.example.com",
        "ftp://example.com/resource",
        "htp:/invalid-url" // Malformed URL
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("--- Running C Test Cases ---\n");
    for (int i = 0; i < num_tests; i++) {
        printf("Input : %s\n", testUrls[i]);
        printf("Output: %s\n", redirectInvalidUrl(testUrls[i]));
        printf("---------------------------------\n");
    }

    return 0;
}