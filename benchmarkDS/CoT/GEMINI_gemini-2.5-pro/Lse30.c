#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
 * @brief Determines the redirect URL based on a target parameter.
 * 
 * The regular expression matches a string that starts with one or more 
 * alphanumeric characters or hyphens, followed by a period.
 * This function uses POSIX regular expressions.
 * 
 * NOTE: The caller is responsible for freeing the memory of the returned string.
 * 
 * @param target The target query parameter string.
 * @return A newly allocated string containing the target if it's valid, 
 *         or "/" if it's invalid. Returns NULL on allocation failure.
 */
char* getRedirectUrl(const char* target) {
    // The regex ensures the target starts like a domain name (e.g., "example.com")
    // and is not a scheme-relative URL (e.g., "//evil.com") or an absolute path (e.g., "/login").
    const char* pattern = "^[a-zA-Z0-9-]+\\..*";
    regex_t regex;
    int ret;

    // Check for NULL or empty input, which is considered invalid.
    if (target == NULL || *target == '\0') {
        // strdup is a POSIX function that duplicates a string.
        return strdup("/");
    }

    // Compile the regular expression.
    // REG_EXTENDED allows for modern regex syntax.
    // REG_NOSUB is a performance optimization as we only care about the match, not capture groups.
    ret = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (ret) {
        fprintf(stderr, "Could not compile regex\n");
        // On compilation failure, default to the safe option.
        return strdup("/"); 
    }

    // Execute the regex.
    ret = regexec(&regex, target, 0, NULL, 0);
    
    char* result = NULL;
    if (ret == 0) {
        // Match found: the target is safe.
        result = strdup(target);
    } else if (ret == REG_NOMATCH) {
        // No match: redirect to root.
        result = strdup("/");
    } else {
        // An error occurred during matching.
        char msgbuf[100];
        regerror(ret, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        result = strdup("/"); // Default to safe option.
    }

    // Free the compiled regular expression.
    regfree(&regex);

    return result;
}

int main() {
    const char* testCases[] = {
        "example.com/home",          // Valid: Standard domain with path
        "sub-domain.net?query=1",    // Valid: Subdomain with query string
        "/local/path",               // Invalid: Does not start with a domain-like pattern
        "justaword",                 // Invalid: No period
        "@bad.com/path"              // Invalid: Starts with a disallowed character
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < numTestCases; ++i) {
        char* result = getRedirectUrl(testCases[i]);
        if (result != NULL) {
            printf("Input: \"%s\", Redirect To: \"%s\"\n", testCases[i], result);
            // The caller must free the memory allocated by getRedirectUrl.
            free(result);
        } else {
            fprintf(stderr, "Memory allocation failed for input: %s\n", testCases[i]);
        }
    }

    return 0;
}