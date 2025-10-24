#include <stdio.h>
#include <string.h>

// Define a structure to hold the key-value pairs for our allowlist.
typedef struct {
    const char* key;
    const char* url;
} RedirectMapping;

// A secure, allowlisted array of page keys to their full URLs.
static const RedirectMapping ALLOWED_REDIRECTS[] = {
    {"home", "https://example.com/home"},
    {"about", "https://example.com/about"},
    {"contact", "https://example.com/contact-us"},
    {"products", "https://example.com/products"},
    {NULL, NULL} // Sentinel value to mark the end of the array
};

static const char* DEFAULT_URL = "https://example.com/error";

/**
 * Securely gets a redirect URL based on a predefined allowlist.
 * Prevents open redirect vulnerabilities by iterating through a safe list.
 *
 * @param pageKey The user-provided key for the desired page.
 * @return The corresponding URL from the allowlist, or a default URL if the key is not found.
 */
const char* getRedirectUrl(const char* pageKey) {
    if (pageKey == NULL) {
        return DEFAULT_URL;
    }

    for (int i = 0; ALLOWED_REDIRECTS[i].key != NULL; ++i) {
        if (strcmp(pageKey, ALLOWED_REDIRECTS[i].key) == 0) {
            return ALLOWED_REDIRECTS[i].url;
        }
    }

    return DEFAULT_URL;
}

int main() {
    const char* testCases[] = {
        "home",           // Test case 1: Valid input
        "products",       // Test case 2: Another valid input
        "admin",          // Test case 3: Invalid input
        "http://evil.com",// Test case 4: Malicious input attempt
        NULL              // Test case 5: Null input
    };

    printf("Running C Test Cases:\n");
    for (int i = 0; i < 5; ++i) {
        const char* redirectUrl = getRedirectUrl(testCases[i]);
        printf("Input: '%s' -> Redirecting to: %s\n", testCases[i] ? testCases[i] : "NULL", redirectUrl);
    }

    return 0;
}