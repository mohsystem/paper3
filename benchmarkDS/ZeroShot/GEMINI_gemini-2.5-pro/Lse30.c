#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

/**
 * Determines the redirect URL based on a regex pattern.
 * WARNING: The logic described in the prompt is vulnerable to Open Redirect attacks.
 * A user can be redirected to any external domain that matches the pattern (e.g., "evil.com").
 * A more secure implementation should validate the domain against a strict allow-list.
 *
 * @param target The target URL, supposedly from a query parameter.
 * @return A const char* to the redirect URL (either the original target or "/").
 *         The returned pointer does NOT need to be freed by the caller.
 */
const char* getRedirectUrl(const char* target) {
    if (target == NULL || *target == '\0') {
        printf("Target is null or empty. Redirecting to root.\n");
        return "/";
    }
    
    regex_t regex;
    int status;
    // Regex: matches a string that starts with one or more alphanumeric characters or hyphens,
    // followed by a period.
    const char* pattern = "^[a-zA-Z0-9-]+\\..*";
    
    // Compile the regular expression
    status = regcomp(&regex, pattern, REG_EXTENDED);
    if (status) {
        fprintf(stderr, "Error: Could not compile regex.\n");
        // Default to a safe redirect on critical error
        return "/";
    }
    
    // Execute the regular expression
    status = regexec(&regex, target, 0, NULL, 0);
    
    const char* redirectUrl;
    if (status == 0) {
        // Match found
        printf("Target '%s' matches the pattern. Redirecting to target.\n", target);
        redirectUrl = target;
    } else if (status == REG_NOMATCH) {
        // No match found
        printf("Target '%s' does not match the pattern. Redirecting to root.\n", target);
        redirectUrl = "/";
    } else {
        // Some other error occurred
        char msgbuf[100];
        regerror(status, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Error: Regex match failed: %s\n", msgbuf);
        // Default to a safe redirect on error
        redirectUrl = "/";
    }
    
    // Free the compiled regular expression
    regfree(&regex);
    
    return redirectUrl;
}

int main() {
    const char* testCases[] = {
        "dashboard.myapp.com/user/1", // Valid
        "some-service.net/api/v1",    // Valid
        "example.com",                // Valid
        "/settings/profile",          // Invalid
        "malicious.com"               // Valid (demonstrates open redirect risk)
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < numTestCases; i++) {
        const char* testCase = testCases[i];
        const char* redirectUrl = getRedirectUrl(testCase);
        printf("=> Final Redirect URL: %s\n", redirectUrl);
        printf("-------------------------\n");
    }

    return 0;
}