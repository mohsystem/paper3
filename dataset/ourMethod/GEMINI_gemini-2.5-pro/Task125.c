#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stddef.h>

// A predefined list of allowed resources.
static const char* ALLOWED_RESOURCES[] = {
    "PROFILE", "DASHBOARD", "SETTINGS", "REPORTS"
};
// Calculate the number of items in the array at compile time.
static const int NUM_ALLOWED_RESOURCES = sizeof(ALLOWED_RESOURCES) / sizeof(ALLOWED_RESOURCES[0]);

/**
 * @brief Checks resource access and writes the result to a caller-provided buffer.
 * This function validates inputs and uses safe string operations to prevent
 * buffer overflows and other vulnerabilities.
 *
 * @param resourceId The ID of the resource being requested.
 * @param outputBuffer The buffer to write the result message to.
 * @param bufferSize The size of the output buffer.
 */
void getResourceAccess(const char* resourceId, char* outputBuffer, size_t bufferSize) {
    // Rule#6: Ensure all input is validated. Check for null pointers and sufficient buffer size.
    if (outputBuffer == NULL || bufferSize == 0) {
        return; // Cannot write output if buffer is invalid.
    }
    // Rule#8: Initialize buffer to be a valid empty C-string to prevent errors.
    outputBuffer[0] = '\0';
    
    if (resourceId == NULL || resourceId[0] == '\0') {
        // Rule#8: Use snprintf for safe, bounded string formatting.
        snprintf(outputBuffer, bufferSize, "Access Denied: Invalid resource ID.");
        return;
    }

    bool accessGranted = false;
    for (int i = 0; i < NUM_ALLOWED_RESOURCES; ++i) {
        // Use strcmp for safe string comparison.
        if (strcmp(resourceId, ALLOWED_RESOURCES[i]) == 0) {
            accessGranted = true;
            break;
        }
    }

    if (accessGranted) {
        snprintf(outputBuffer, bufferSize, "Access Granted to resource: %s", resourceId);
    } else {
        // Return a generic error to avoid information disclosure.
        snprintf(outputBuffer, bufferSize, "Access Denied: Resource not found or permission denied.");
    }
}

int main() {
    // A buffer on the stack to hold the result message.
    char result[256];

    printf("--- Running Test Cases ---\n");

    // Test Case 1: A valid, allowed resource.
    const char* resource1 = "DASHBOARD";
    getResourceAccess(resource1, result, sizeof(result));
    printf("Requesting '%s': %s\n", resource1, result);

    // Test Case 2: Another valid, allowed resource.
    const char* resource2 = "SETTINGS";
    getResourceAccess(resource2, result, sizeof(result));
    printf("Requesting '%s': %s\n", resource2, result);

    // Test Case 3: An invalid/unauthorized resource.
    const char* resource3 = "ADMIN_PANEL";
    getResourceAccess(resource3, result, sizeof(result));
    printf("Requesting '%s': %s\n", resource3, result);

    // Test Case 4: An empty string input.
    const char* resource4 = "";
    getResourceAccess(resource4, result, sizeof(result));
    printf("Requesting 'empty string': %s\n", result);

    // Test Case 5: A null input.
    const char* resource5 = NULL;
    getResourceAccess(resource5, result, sizeof(result));
    printf("Requesting null: %s\n", result);

    printf("--- Test Cases Finished ---\n");

    return 0;
}