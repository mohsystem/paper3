#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Define a structure to map a user-friendly key to a safe internal path.
typedef struct {
    const char* key;
    const char* path;
} ResourceMapping;

// A whitelist of allowed resources. This is the core of the secure approach.
// Using a predefined list prevents any form of path traversal or injection.
static const ResourceMapping allowedResources[] = {
    {"profile", "data/user_profile.dat"},
    {"settings", "config/app_settings.json"},
    {"dashboard", "pages/dashboard.html"},
    {NULL, NULL} // Sentinel to mark the end of the array
};

/**
 * Securely accesses a resource based on a predefined whitelist.
 *
 * @param resourceKey The user-provided key for the resource.
 * @param outputBuffer A buffer to write the result message into.
 * @param bufferSize The size of the output buffer.
 */
void accessResource(const char* resourceKey, char* outputBuffer, size_t bufferSize) {
    // Input validation: check for NULL or empty string.
    if (resourceKey == NULL || *resourceKey == '\0') {
        snprintf(outputBuffer, bufferSize, "Error: Resource key cannot be empty.");
        return;
    }

    // Iterate through the whitelist to find a match.
    for (int i = 0; allowedResources[i].key != NULL; ++i) {
        // Use strcmp for a safe, direct comparison.
        if (strcmp(resourceKey, allowedResources[i].key) == 0) {
            // Found a valid resource.
            // In a real application, you would use the safe path.
            snprintf(outputBuffer, bufferSize, "Success: Accessing resource at %s", allowedResources[i].path);
            return;
        }
    }

    // If no match is found, return a generic error message.
    snprintf(outputBuffer, bufferSize, "Error: Resource not found or access denied.");
}

int main() {
    printf("C Test Cases:\n");

    const char* testCases[] = {
        "profile",          // Valid case 1
        "settings",         // Valid case 2
        "logs",             // Invalid case
        "../etc/passwd",    // Path traversal attempt
        ""                  // Empty input
    };
    
    char resultBuffer[256];
    int numTests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTests; ++i) {
        accessResource(testCases[i], resultBuffer, sizeof(resultBuffer));
        printf("Input: \"%s\" -> Output: %s\n", testCases[i], resultBuffer);
    }

    return 0;
}