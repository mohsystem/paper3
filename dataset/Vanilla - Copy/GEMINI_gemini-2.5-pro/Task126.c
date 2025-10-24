#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * Redirects a user based on a page string.
 * This function writes the result message into a provided buffer.
 *
 * @param page The input string representing the desired page.
 * @param resultBuffer The character buffer to store the output message.
 * @param bufferSize The size of the resultBuffer to prevent overflow.
 */
void redirectUser(const char* page, char* resultBuffer, size_t bufferSize) {
    // Create a mutable copy to convert to lowercase
    char lowerPage[100];
    strncpy(lowerPage, page, sizeof(lowerPage) - 1);
    lowerPage[sizeof(lowerPage) - 1] = '\0'; // Ensure null termination

    for(int i = 0; lowerPage[i]; i++){
      lowerPage[i] = tolower(lowerPage[i]);
    }
    
    if (strcmp(lowerPage, "home") == 0) {
        snprintf(resultBuffer, bufferSize, "Redirecting to Home Page...");
    } else if (strcmp(lowerPage, "about") == 0) {
        snprintf(resultBuffer, bufferSize, "Redirecting to About Us Page...");
    } else if (strcmp(lowerPage, "contact") == 0) {
        snprintf(resultBuffer, bufferSize, "Redirecting to Contact Page...");
    } else if (strcmp(lowerPage, "products") == 0) {
        snprintf(resultBuffer, bufferSize, "Redirecting to Products Page...");
    } else {
        snprintf(resultBuffer, bufferSize, "Page not found. Redirecting to 404 Error Page...");
    }
}

int main() {
    const char* testCases[] = {"home", "about", "contact", "products", "login"};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);
    char result[100];

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < numTestCases; i++) {
        redirectUser(testCases[i], result, sizeof(result));
        printf("Input: \"%s\" -> Output: %s\n", testCases[i], result);
    }

    return 0;
}