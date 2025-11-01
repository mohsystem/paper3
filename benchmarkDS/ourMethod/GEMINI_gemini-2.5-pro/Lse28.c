#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <regex.h>

// Global state for our "redirector module"
static regex_t safe_url_regex;
static int is_regex_compiled = 0;
static const char* DEFAULT_URL = "https://default.example.com";

// Forward declaration for cleanup function
void cleanup_redirector_regex(void);

/**
 * @brief Compiles the global regular expression.
 * This function should be called once at program startup.
 * It sets up a cleanup function to be called at program exit.
 */
void initialize_redirector(void) {
    if (is_regex_compiled) {
        return;
    }
    const char* pattern = "^https://safe\\.example\\.com/.*$";
    // REG_EXTENDED for modern regex syntax, REG_NOSUB for performance as we only need a match/no-match status.
    int result = regcomp(&safe_url_regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (result != 0) {
        char error_message[100];
        regerror(result, &safe_url_regex, error_message, sizeof(error_message));
        fprintf(stderr, "Fatal: Could not compile regex: %s\n", error_message);
        exit(EXIT_FAILURE);
    }
    is_regex_compiled = 1;
    // Ensure the compiled regex is freed when the program exits.
    atexit(cleanup_redirector_regex);
}

/**
 * @brief Frees the memory used by the compiled regular expression.
 * Registered by `initialize_redirector` to be called via `atexit`.
 */
void cleanup_redirector_regex(void) {
    if (is_regex_compiled) {
        regfree(&safe_url_regex);
        is_regex_compiled = 0;
    }
}

/**
 * Determines the redirect URL based on a whitelist pattern.
 *
 * @param target_url The URL to check.
 * @return The target_url if it matches the safe pattern, otherwise the default URL.
 *         The returned pointer is to either the input `target_url` or the global
 *         `DEFAULT_URL` and should not be freed by the caller.
 */
const char* get_redirect_url(const char* target_url) {
    // Fail safe if the regex engine was not initialized.
    if (!is_regex_compiled) {
        fprintf(stderr, "Warning: Redirector not initialized. Returning default URL.\n");
        return DEFAULT_URL;
    }

    // Rule #1: Validate input. Check for NULL or empty strings.
    if (target_url == NULL || *target_url == '\0') {
        return DEFAULT_URL;
    }

    // Execute the regular expression match.
    int result = regexec(&safe_url_regex, target_url, 0, NULL, 0);

    if (result == 0) {
        // A match was found. The URL is safe.
        return target_url;
    } else if (result == REG_NOMATCH) {
        // No match found. Return the default URL.
        return DEFAULT_URL;
    } else {
        // An error occurred during regex execution.
        char error_message[100];
        regerror(result, &safe_url_regex, error_message, sizeof(error_message));
        fprintf(stderr, "Regex match failed: %s\n", error_message);
        return DEFAULT_URL; // Fail safe
    }
}

int main(void) {
    // Compile the regex once when the application starts.
    initialize_redirector();

    const char* test_urls[] = {
        // Test Case 1: Valid URL that should be allowed.
        "https://safe.example.com/user/profile",
        // Test Case 2: URL from an untrusted domain.
        "https://unsafe.example.com/user/profile",
        // Test Case 3: URL with an insecure protocol.
        "http://safe.example.com/user/profile",
        // Test Case 4: Empty input string.
        "",
        // Test Case 5: NULL input.
        NULL
    };
    int num_tests = sizeof(test_urls) / sizeof(test_urls[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        const char* url = test_urls[i];
        const char* redirect_url = get_redirect_url(url);
        printf("Input:  \"%s\"\nOutput: \"%s\"\n\n", url ? url : "NULL", redirect_url);
    }

    return 0;
}