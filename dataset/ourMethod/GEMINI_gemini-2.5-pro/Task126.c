#include <stdio.h>
#include <string.h>

/**
 * Returns a redirect URL based on a predefined safe list.
 * This prevents open redirect vulnerabilities by never using the input
 * to construct the URL. It compares the input against a hardcoded
 * list of safe destinations.
 *
 * @param pageKey A string key representing the desired page.
 * @return A constant string literal representing a hardcoded, safe URL.
 */
const char* getRedirectUrl(const char* pageKey) {
    const char* defaultUrl = "https://example.com/404";

    if (pageKey == NULL || pageKey[0] == '\0') {
        return defaultUrl;
    }

    if (strcmp(pageKey, "home") == 0) {
        return "https://example.com/home";
    }
    if (strcmp(pageKey, "profile") == 0) {
        return "https://example.com/user/profile";
    }
    if (strcmp(pageKey, "settings") == 0) {
        return "https://example.com/user/settings";
    }
    if (strcmp(pageKey, "login") == 0) {
        return "https://example.com/auth/login";
    }

    return defaultUrl;
}

int main() {
    // Test cases
    const char* testInputs[] = {
        "home",
        "settings",
        "dashboard", // Invalid key
        "",          // Empty key
        "//malicious-site.com" // Malicious input attempt
    };
    int numTests = sizeof(testInputs) / sizeof(testInputs[0]);

    printf("Running test cases for getRedirectUrl:\n");
    for (int i = 0; i < numTests; ++i) {
        printf("Input: \"%s\" -> Redirect URL: %s\n", testInputs[i], getRedirectUrl(testInputs[i]));
    }

    // Test case for null input
    printf("Input: NULL -> Redirect URL: %s\n", getRedirectUrl(NULL));

    return 0;
}