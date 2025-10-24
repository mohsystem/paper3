#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Validates if a given string is a well-formed URL (simplified check).
 * This function performs a basic validation, checking for a valid scheme
 * and a non-empty authority part. It does not use regular expressions.
 *
 * @param urlString The string to validate.
 * @return true if the string is a valid URL, false otherwise.
 */
bool isValidURL(const char* urlString) {
    if (urlString == NULL) {
        return false;
    }

    const char* authority_start = NULL;

    // Check for a scheme and find the start of the authority
    if (strncmp(urlString, "https://", 8) == 0) {
        authority_start = urlString + 8;
    } else if (strncmp(urlString, "http://", 7) == 0) {
        authority_start = urlString + 7;
    } else if (strncmp(urlString, "ftp://", 6) == 0) {
        authority_start = urlString + 6;
    } else {
        return false; // No valid scheme found
    }

    // The authority part cannot be empty.
    if (*authority_start == '\0') {
        return false;
    }

    // The authority part must not contain whitespace and must have at least one character
    // before a path starts.
    const char *p = authority_start;
    bool has_authority_char = false;
    while (*p != '\0' && *p != '/' && *p != '?' && *p != '#') {
        if (isspace((unsigned char)*p)) {
            return false; // Whitespace in authority is invalid
        }
        has_authority_char = true;
        p++;
    }

    return has_authority_char;
}

int main() {
    const char* testUrls[] = {
        "https://www.google.com",
        "http://example.com/path?query=1",
        "ftp://ftp.is.co.za/rfc/rfc1808.txt",
        "www.invalid.com",
        "https://",
        "http://exa mple.com",
        NULL // Test case for NULL input
    };
    int numTests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("Running URL validation tests:\n");
    for (int i = 0; i < numTests; i++) {
        const char* url = testUrls[i];
        if (url == NULL) {
            printf("URL: \"(null)\" -> Valid: %s\n", isValidURL(url) ? "true" : "false");
        } else {
            printf("URL: \"%s\" -> Valid: %s\n", url, isValidURL(url) ? "true" : "false");
        }
    }

    return 0;
}