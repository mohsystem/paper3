#include <stdio.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>

// The default URL to use if the target URL does not match the whitelist.
const char* DEFAULT_URL = "https://example.com/default";

// Global static regex_t object and a flag to ensure it's compiled only once.
static regex_t safe_url_regex;
static int regex_compiled = 0;

// Function to free the compiled regex, registered with atexit.
void cleanup_regex() {
    if (regex_compiled) {
        regfree(&safe_url_regex);
    }
}

// Function to compile the regex. It's called only once.
void compile_regex_once() {
    if (!regex_compiled) {
        // The regular expression whitelists URLs from a specific, safe domain.
        const char* pattern = "^https://safe\\.example\\.com/([a-zA-Z0-9/\\-]+)$";
        if (regcomp(&safe_url_regex, pattern, REG_EXTENDED | REG_NOSUB) != 0) {
            fprintf(stderr, "Could not compile regex\n");
            exit(EXIT_FAILURE);
        }
        regex_compiled = 1;
        // Register cleanup function to be called at program exit.
        atexit(cleanup_regex);
    }
}

/**
 * Determines the redirect URL based on a whitelist pattern.
 * If the target URL matches the pre-compiled regular expression, it is returned.
 * Otherwise, a default URL is returned.
 *
 * @param targetUrl The URL to validate for redirection.
 * @return The targetUrl if it's valid, otherwise the default URL.
 */
const char* getRedirectUrl(const char* targetUrl) {
    // Ensure the regex is compiled before first use.
    compile_regex_once();

    if (targetUrl == NULL) {
        return DEFAULT_URL;
    }
    
    // The regular expression is used to match the target URL.
    int status = regexec(&safe_url_regex, targetUrl, 0, NULL, 0);

    if (status == 0) {
        // If the target URL matches, it is used as the redirect target.
        // The regex is not used to construct the URL, preventing manipulation.
        return targetUrl;
    } else {
        // If the target URL does not match, a default URL is used.
        return DEFAULT_URL;
    }
}

// In C, the main class Lse28 is not applicable.
// The code is organized into functions.
int main() {
    const char* testUrls[] = {
        // Test Case 1: Valid URL
        "https://safe.example.com/valid-path",
        // Test Case 2: Valid URL with sub-path
        "https://safe.example.com/user/profile-123",
        // Test Case 3: Invalid domain
        "https://unsafe.example.com/path",
        // Test Case 4: Invalid characters (query parameters are not allowed by the regex)
        "https://safe.example.com/path?query=malicious",
        // Test Case 5: URL trying to bypass with different protocol or domain format
        "http://safe.example.com/path"
    };
    
    int num_tests = sizeof(testUrls) / sizeof(testUrls[0]);
    
    printf("C Test Cases:\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Input:  %s\n", testUrls[i]);
        printf("Output: %s\n", getRedirectUrl(testUrls[i]));
        printf("--------------------\n");
    }
    
    return 0;
}