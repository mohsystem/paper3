#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// A struct to hold resource key-value pairs
typedef struct {
    const char* name;
    const char* content;
} Resource;

// Our resource "database"
static const Resource resources[] = {
    {"profile", "User Profile Data: { 'user': 'John Doe', 'email': 'john.doe@example.com' }"},
    {"settings", "Account Settings Page: [ 'Theme: Dark', 'Language: English' ]"},
    {"dashboard", "Main Dashboard View: Widgets and Stats"},
    {"logs", "System Activity Logs: [ 'INFO: User login', 'WARN: Low disk space' ]"},
    {NULL, NULL} // Sentinel to mark the end of the array
};

/**
 * Accesses a resource based on the provided resource name.
 * @param resourceName The name of the resource to access.
 * @return A dynamically allocated string with the result. The caller must free this string.
 */
char* getResource(const char* resourceName) {
    if (resourceName == NULL) {
        return NULL;
    }

    // Create a lowercase version of the input for case-insensitive comparison
    char* lowerResourceName = strdup(resourceName);
    for (int i = 0; lowerResourceName[i]; i++) {
        lowerResourceName[i] = tolower((unsigned char)lowerResourceName[i]);
    }

    const char* foundContent = NULL;
    for (int i = 0; resources[i].name != NULL; ++i) {
        if (strcmp(resources[i].name, lowerResourceName) == 0) {
            foundContent = resources[i].content;
            break;
        }
    }
    
    free(lowerResourceName);

    // Allocate buffer for the output string
    char* result;
    if (foundContent) {
        // Size = "Accessing " + resourceName + ": " + content + null terminator
        size_t len = strlen("Accessing : ") + strlen(resourceName) + strlen(foundContent) + 1;
        result = (char*)malloc(len);
        sprintf(result, "Accessing %s: %s", resourceName, foundContent);
    } else {
        // Size = "Access Denied: Resource '' not found." + resourceName + null terminator
        size_t len = strlen("Access Denied: Resource '' not found.") + strlen(resourceName) + 1;
        result = (char*)malloc(len);
        sprintf(result, "Access Denied: Resource '%s' not found.", resourceName);
    }

    return result;
}

int main() {
    // Test Cases
    const char* testCases[] = {"profile", "dashboard", "admin", "settings", "data"};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    
    printf("--- C Test Cases ---\n");
    for (int i = 0; i < numTestCases; ++i) {
        printf("Input: %s\n", testCases[i]);
        char* result = getResource(testCases[i]);
        if (result != NULL) {
            printf("Output: %s\n\n", result);
            free(result); // Free the memory allocated by getResource
        }
    }

    return 0;
}