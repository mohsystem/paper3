#include <stdio.h>
#include <string.h>
#include <ctype.h>

/**
 * @brief Trims trailing whitespace from a string in-place.
 *
 * This function is secure:
 * - It checks for a NULL pointer to prevent segfaults.
 * - It safely modifies the string by moving the null terminator,
 *   which does not risk buffer overflows.
 * - It uses isspace() on an unsigned char to avoid undefined behavior.
 *
 * @param str A pointer to the character array (string) to be trimmed.
 *            The string must be mutable.
 */
void trimTrailingWhitespace(char *str) {
    if (str == NULL) {
        return; // Safety check for null pointers
    }

    int len = strlen(str);
    if (len == 0) {
        return; // Nothing to do for an empty string
    }

    char *end = str + len - 1;

    // Move backwards from the end of the string, checking for whitespace
    while (end >= str && isspace((unsigned char)*end)) {
        end--;
    }

    // Place a new null-terminator after the last non-whitespace character
    *(end + 1) = '\0';
}

int main() {
    // Test cases must be mutable character arrays, not string literals,
    // because the function modifies them in-place.
    char test1[] = "hello world   ";
    char test2[] = "test\t \t";
    char test3[] = "no_trailing";
    char test4[] = "   \t\n ";
    char test5[] = "  leading and trailing  ";
    
    // Array of pointers to the test cases
    char* testCases[] = {test1, test2, test3, test4, test5};
    // Keep original strings for printing, since testCases will be modified
    const char* originalStrings[] = {
        "hello world   ",
        "test\t \t",
        "no_trailing",
        "   \t\n ",
        "  leading and trailing  "
    };
    int numTests = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running C test cases:\n");
    for (int i = 0; i < numTests; i++) {
        // Print original state
        printf("Input:  \"%s\"\n", originalStrings[i]);
        
        // Trim the string
        trimTrailingWhitespace(testCases[i]);
        
        // Print modified state
        printf("Output: \"%s\"\n\n", testCases[i]);
    }

    return 0;
}