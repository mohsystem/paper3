#include <stdio.h>
#include <string.h>
#include <regex.h> // For POSIX regular expressions

// Global variables to hold the compiled regex and default URL
static regex_t url_regex;
static int regex_compiled = 0;
static const char* DEFAULT_URL = "https://default-redirect.com";

/**
 * Compiles the regex pattern. Should be called once.
 */
void compile_regex() {
    if (!regex_compiled) {
        // The regular expression is compiled once.
        const char* pattern = "^https?://(www\\.)?example\\.com/.*$";
        if (regcomp(&url_regex, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
            fprintf(stderr, "Could not compile regex\n");
            // In a real app, you would handle this error more gracefully.
        } else {
            regex_compiled = 1;
        }
    }
}

/**
 * Determines the redirect URL based on a pattern match.
 *
 * @param targetUrl The URL to check.
 * @return The targetUrl if it matches the pattern, otherwise the default URL.
 *         The returned pointer refers to existing memory and should not be freed.
 */
const char* getRedirectUrl(const char* targetUrl) {
    if (regex_compiled && targetUrl != NULL) {
        // regexec returns 0 on a successful match.
        // If the target URL matches, it is used as the redirect target.
        if (regexec(&url_regex, targetUrl, 0, NULL, 0) == 0) {
            return targetUrl;
        }
    }
    // If the target URL does not match, a default URL is used.
    return DEFAULT_URL;
}

int main() {
    // Compile the regex before using it.
    compile_regex();

    const char* testUrls[] = {
        "https://www.example.com/path/to/resource", // Should match
        "http://example.com/another-page",          // Should match
        "https://www.google.com",                   // Should not match
        "https://subdomain.example.com/page",       // Should not match
        "ftp://example.com/file"                    // Should not match
    };

    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < num_tests; ++i) {
        const char* redirectUrl = getRedirectUrl(testUrls[i]);
        printf("Input: %s -> Redirect: %s\n", testUrls[i], redirectUrl);
    }
    
    // Free the compiled regular expression
    if (regex_compiled) {
        regfree(&url_regex);
    }

    return 0;
}