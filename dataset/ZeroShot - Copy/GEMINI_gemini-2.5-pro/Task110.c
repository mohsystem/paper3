#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

/**
 * Validates a URL using the POSIX regex library.
 * The approach is similar to the C++ version, using a carefully crafted regex
 * to validate the URL structure securely. It includes a length check to
 * prevent excessive resource usage.
 *
 * @param urlString The URL string to validate.
 * @return 1 if the URL is valid, 0 otherwise.
 */
int validateUrl(const char* urlString) {
    // 1. Check for NULL pointer and unreasonable length.
    if (urlString == NULL || strlen(urlString) > 2048) {
        return 0; // 0 for invalid
    }

    regex_t regex;
    int result;

    // 2. A practical POSIX ERE-compatible regex for URL validation.
    // Note: C string literals require escaping backslashes.
    const char* pattern = "^(https?|ftp)://"
                          "(([a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\\.)+[a-zA-Z]{2,63}|"
                          "localhost|"
                          "((\\d{1,3}\\.){3}\\d{1,3}))"
                          "(:\\d{1,5})?"
                          "(/[^?#]*)?"
                          "(\\?[^#]*)?"
                          "(#.*)?$";

    // 3. Compile the regular expression.
    // REG_EXTENDED for modern regex syntax.
    // REG_ICASE for case-insensitivity.
    if (regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE) != 0) {
        fprintf(stderr, "Could not compile regex\n");
        return 0;
    }

    // 4. Execute the regex match.
    result = regexec(&regex, urlString, 0, NULL, 0);

    // 5. Free the memory used by the compiled regex.
    regfree(&regex);

    // regexec returns 0 on a successful match.
    return (result == 0);
}

int main() {
    const char* testUrls[] = {
        "https://www.google.com",
        "http://example.com/path?name=value#fragment",
        "ftp://127.0.0.1:21/resource",
        "www.google.com", // Invalid: no scheme
        "http://invalid-.com", // Invalid: domain label ends with hyphen
        "garbage" // Invalid: not a URL
    };
    int numTests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("C URL Validation:\n");
    for (int i = 0; i < numTests; i++) {
        printf("URL: \"%s\" -> %s\n", testUrls[i], validateUrl(testUrls[i]) ? "Valid" : "Invalid");
    }

    return 0;
}