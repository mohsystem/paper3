#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <regex.h> // For POSIX regular expressions

/**
 * Validates a URL using a regular expression.
 * This regex checks for http, https, or ftp schemes, a valid domain name,
 * and an optional path.
 *
 * @param url The string to validate.
 * @return true if the URL is valid, false otherwise.
 */
bool isValidURL(const char* url) {
    // Handle NULL or empty input
    if (url == NULL || url[0] == '\0') {
        return false;
    }
    
    regex_t regex;
    int reti;
    bool is_match = false;

    // Regex for a standard URL (http, https, ftp)
    // ^(https?|ftp)://    - Scheme (http, https, ftp)
    // [a-zA-Z0-9.-]+      - Domain name characters
    // \.[a-zA-Z]{2,}      - Top-level domain (at least 2 letters)
    // (/[^[:space:]]*)?$  - Optional path (any non-whitespace characters)
    // NOTE: using POSIX character class [[:space:]] instead of \s
    const char* pattern = "^(https?|ftp)://[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}(/[^[:space:]]*)?$";

    // Compile the regular expression
    reti = regcomp(&regex, pattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return false;
    }

    // Execute the regular expression
    reti = regexec(&regex, url, 0, NULL, 0);
    if (reti == 0) {
        is_match = true; // Match found
    } else if (reti == REG_NOMATCH) {
        is_match = false; // No match
    } else {
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        is_match = false;
    }

    // Free the compiled regular expression
    regfree(&regex);

    return is_match;
}

int main() {
    const char* testUrls[] = {
        "https://www.example.com",
        "http://example.co.uk/path/to/page?query=123",
        "ftp://ftp.example.com/resource",
        "htp://invalid-scheme.com", // Invalid scheme
        "https://.invalid-domain.com", // Invalid domain
        "https://domain-without-tld", // Invalid TLD
        "Just a string", // Not a URL
        "" // Empty string
    };
    
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("Running URL validation test cases in C:\n");
    for (int i = 0; i < num_tests; i++) {
        printf("URL: \"%s\" -> %s\n", testUrls[i], isValidURL(testUrls[i]) ? "Valid" : "Invalid");
    }

    return 0;
}