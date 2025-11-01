#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

// NOTE: This logic is vulnerable to CWE-601: Open Redirect.
// A user can be redirected to any external site that matches this pattern (e.g., "evil-site.com").
// A safer implementation would validate the target against a whitelist of allowed domains or paths.

/**
 * @brief Determines the redirect URL based on a target parameter.
 *
 * @param target The target parameter from the request.
 * @return The target URL if it's valid, otherwise the root path "/".
 *         The returned pointer is either to the original target string or a static literal,
 *         and should NOT be freed by the caller.
 */
const char* getRedirectUrl(const char* target) {
    // Rule #1: Ensure all input is validated.
    // A NULL or empty target is considered invalid.
    if (target == NULL || *target == '\0') {
        return "/";
    }

    regex_t regex;
    int reti;
    // The regex matches a string that starts with one or more alphanumeric characters
    // or hyphens, followed by a period.
    const char* pattern = "^[a-zA-Z0-9-]+\\..*";

    // Compile the regular expression
    // Using REG_EXTENDED for modern regex syntax, REG_NOSUB to slightly optimize as we don't need sub-expressions.
    reti = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (reti) {
        // This is a server-side configuration error. Log it and fail safely.
        fprintf(stderr, "Could not compile regex\n");
        return "/"; // Fail closed
    }

    // Execute the regex
    reti = regexec(&regex, target, 0, NULL, 0);

    // Free the compiled regex object
    regfree(&regex);

    if (reti == 0) {
        // Match
        return target;
    } else if (reti == REG_NOMATCH) {
        // No match
        return "/";
    } else {
        // An error occurred during matching. Log and fail safely.
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        return "/"; // Fail closed
    }
}

int main() {
    const char* testCases[] = {
        "example.com",                  // Valid
        "sub-domain.net/path?query=1",  // Valid
        "nodot",                        // Invalid: no dot
        ".startwithdot.com",            // Invalid: starts with dot
        "",                             // Invalid: empty string
        NULL                            // Invalid: null
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running C test cases...\n");
    for (int i = 0; i < numTestCases; i++) {
        const char* target = testCases[i];
        const char* redirectUrl = getRedirectUrl(target);
        printf("Test Case %d: target=\"%s\", redirect_to=\"%s\"\n", i + 1, target, redirectUrl);
    }

    return 0;
}