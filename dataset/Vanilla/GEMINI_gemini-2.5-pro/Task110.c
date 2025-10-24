#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/**
 * Validates a URL string by manually parsing it.
 * Checks for http/https protocol and a simple, valid domain structure.
 * @param url The URL string to validate.
 * @return true if the URL is valid, false otherwise.
 */
bool isValidURL(const char* url) {
    if (url == NULL) {
        return false;
    }

    // 1. Check for protocol "http://" or "https://"
    const char* domain_start = NULL;
    if (strncmp(url, "http://", 7) == 0) {
        domain_start = url + 7;
    } else if (strncmp(url, "https://", 8) == 0) {
        domain_start = url + 8;
    } else {
        return false;
    }

    // 2. Find the end of the domain part (before a path or query)
    const char* domain_end = strpbrk(domain_start, "/?#");
    if (domain_end == NULL) {
        domain_end = url + strlen(url);
    }
    
    int domain_len = domain_end - domain_start;
    if (domain_len == 0) {
        return false;
    }

    // 3. Domain cannot start or end with '.' or '-'
    if (domain_start[0] == '.' || domain_start[0] == '-' ||
        domain_start[domain_len - 1] == '.' || domain_start[domain_len - 1] == '-') {
        return false;
    }

    bool dot_found = false;
    for (int i = 0; i < domain_len; ++i) {
        char c = domain_start[i];
        
        // 4. Check for valid characters (alphanumeric, dot, hyphen)
        if (!isalnum(c) && c != '.' && c != '-') {
            return false;
        }
        
        // 5. Check for consecutive dots ".."
        if (c == '.' && i > 0 && domain_start[i - 1] == '.') {
            return false;
        }

        if (c == '.') {
            dot_found = true;
        }
    }

    // 6. Domain must contain at least one dot
    return dot_found;
}

int main() {
    const char* testUrls[] = {
        "https://www.google.com",
        "http://example.org/path?name=value",
        "ftp://example.com",
        "https://example..com",
        "http://invalid",
        "just a string"
    };
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("--- C URL Validation ---\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("URL: \"%s\" is %s\n", testUrls[i], isValidURL(testUrls[i]) ? "valid" : "invalid");
    }

    return 0;
}