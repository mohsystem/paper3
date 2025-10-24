#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Redirects a user based on a string choice.
 * The comparison is case-insensitive.
 *
 * @param choice The user's page selection (e.g., "home", "about").
 * @return A const char* to a string literal indicating the URL or an error.
 */
const char* redirectUser(const char* choice) {
    if (choice == NULL || choice[0] == '\0') {
        return "Error: Page not found (404)";
    }

    // Create a buffer for the lowercase version of the input string
    // to perform case-insensitive comparison safely.
    char lower_choice[50];
    int i = 0;

    // Safely copy and convert the input string to lowercase, preventing buffer overflow.
    // The cast to (unsigned char) is important for tolower to avoid undefined behavior.
    while (choice[i] != '\0' && i < sizeof(lower_choice) - 1) {
        lower_choice[i] = tolower((unsigned char)choice[i]);
        i++;
    }
    lower_choice[i] = '\0'; // Null-terminate the new string

    if (strcmp(lower_choice, "home") == 0) {
        return "Redirecting to /home.html";
    } else if (strcmp(lower_choice, "about") == 0) {
        return "Redirecting to /about.html";
    } else if (strcmp(lower_choice, "contact") == 0) {
        return "Redirecting to /contact.html";
    } else if (strcmp(lower_choice, "products") == 0) {
        return "Redirecting to /products.html";
    } else {
        return "Error: Page not found (404)";
    }
}

int main() {
    const char* testCases[] = {"home", "About", "products", "login", ""};
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);
    
    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("Input: \"%s\" -> Output: %s\n", testCases[i], redirectUser(testCases[i]));
    }
    
    return 0;
}