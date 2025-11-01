#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

// This function will be called on program exit to free the compiled regex.
static regex_t compiled_pattern;
static int is_compiled = 0;

void cleanup_regex() {
    if (is_compiled) {
        regfree(&compiled_pattern);
    }
}

/**
 * Determines the redirect URL based on a whitelist pattern.
 * The regex is compiled once when the function is first called.
 * If the target URL matches the pattern, it is returned.
 * Otherwise, a default URL is returned.
 *
 * @param targetUrl The URL to validate.
 * @param defaultUrl The fallback URL if validation fails.
 * @return The validated targetUrl or the defaultUrl. The returned pointer
 *         points to one of the input strings and should not be freed by the caller.
 */
const char* getRedirectUrl(const char* targetUrl, const char* defaultUrl) {
    // This regex validates that the URL is on the example.com domain.
    const char* pattern = "^https?://(www\\.)?example\\.com/.*$";

    // The regular expression is compiled once.
    if (!is_compiled) {
        if (regcomp(&compiled_pattern, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
            // In a real application, log this error.
            // For this example, we'll fail safe and use the default URL.
            return defaultUrl;
        }
        is_compiled = 1;
        // Register the cleanup function to be called at exit.
        atexit(cleanup_regex);
    }

    if (targetUrl == NULL || *targetUrl == '\0') {
        return defaultUrl;
    }
    
    // Execute the regex
    int status = regexec(&compiled_pattern, targetUrl, 0, NULL, 0);
    
    // If the target URL matches (status is 0), it is used.
    if (status == 0) {
        return targetUrl;
    } else {
        // Otherwise, the default URL is used.
        return defaultUrl;
    }
}

int main() {
    const char* defaultUrl = "https://example.com/default";

    // Test Cases
    const char* testUrls[] = {
        "https://example.com/some/path",          // Case 1: Valid HTTPS URL
        "http://www.example.com/another/page.html",// Case 2: Valid HTTP URL with www
        "https://evil.com/some/path",             // Case 3: Invalid domain
        "https://example.com.evil.com/path",      // Case 4: Invalid domain (tricky)
        "not a url"                               // Case 5: Invalid format
    };

    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < num_tests; ++i) {
        const char* target = testUrls[i];
        const char* redirect = getRedirectUrl(target, defaultUrl);
        printf("Test Case %d:\n", i + 1);
        printf("  Target:   %s\n", target);
        printf("  Redirect: %s\n\n", redirect);
    }

    return 0;
}