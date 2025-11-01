#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * @brief Trims trailing whitespace from a string in-place.
 * Whitespace is determined by the isspace() function.
 * @param str The string to trim. It must be a mutable character array.
 */
void trimTrailingWhitespace(char *str) {
    if (str == NULL) {
        return; // Safety check for null pointer
    }
    
    int len = strlen(str);
    if (len == 0) {
        return; // Nothing to do for an empty string
    }

    int i = len - 1;
    // Walk backwards from the end of the string.
    // Cast to unsigned char is a safe practice for ctype functions.
    while (i >= 0 && isspace((unsigned char)str[i])) {
        i--;
    }
    
    // Place the null terminator at the new end of the string
    str[i + 1] = '\0';
}

void runTestCases() {
    printf("\n--- Test Cases ---\n");
    // Test cases must be mutable arrays, not string literals,
    // because the function modifies them in-place.
    char test1[] = "hello world   ";
    char test2[] = "test\t\n ";
    char test3[] = "already_trimmed";
    char test4[] = "      ";
    char test5[] = "";
    
    char* testCases[] = {test1, test2, test3, test4, test5};
    const char* originalStrings[] = {
        "hello world   ",
        "test\t\n ",
        "already_trimmed",
        "      ",
        ""
    };

    for (int i = 0; i < 5; ++i) {
        printf("Test Case %d:\n", i + 1);
        printf("  Original: \"%s\"\n", originalStrings[i]);
        trimTrailingWhitespace(testCases[i]);
        printf("  Trimmed:  \"%s\"\n\n", testCases[i]);
    }
}

int main(int argc, char* argv[]) {
    printf("--- Command Line Argument Processing ---\n");
    if (argc > 1) {
        // Make a mutable copy of the command line argument to safely modify it,
        // as argv strings might be read-only.
        size_t len = strlen(argv[1]);
        char* inputCopy = malloc(len + 1);
        if (inputCopy == NULL) {
            fprintf(stderr, "Error: Memory allocation failed.\n");
            return 1;
        }
        memcpy(inputCopy, argv[1], len + 1); // Use memcpy for safe copying

        printf("Original: \"%s\"\n", inputCopy);
        trimTrailingWhitespace(inputCopy);
        printf("Trimmed:  \"%s\"\n", inputCopy);
        
        free(inputCopy); // Free the allocated memory
    } else {
        // Provide usage instructions if no argument is given
        fprintf(stderr, "Usage: %s \"<string to trim>\"\n", (argc > 0 ? argv[0] : "program_name"));
    }

    runTestCases();

    return 0;
}