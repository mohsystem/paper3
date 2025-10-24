#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * Attempts to convert a string to an integer (long) and prints the result or an error message.
 * Uses strtol for robust error checking.
 * @param s The string to be converted.
 */
void convertStringToInt(const char* s) {
    char* endptr;
    errno = 0; // Reset errno before the call

    long number = strtol(s, &endptr, 10);

    // Check for various possible errors
    // 1. No conversion was performed (e.g., input is "abc")
    if (endptr == s) {
        printf("Input: \"%s\" -> Error: No digits were found.\n", s);
    }
    // 2. The entire string was not consumed (e.g., input is "123xyz")
    else if (*endptr != '\0') {
        printf("Input: \"%s\" -> Error: Trailing characters found: %s\n", s, endptr);
    }
    // 3. The number was out of range for a long
    else if (errno == ERANGE) {
        printf("Input: \"%s\" -> Error: Number out of range for a long integer.\n", s);
    }
    // 4. Success
    else {
        // Optional: Check if the long value fits into a standard int
        if (number > INT_MAX || number < INT_MIN) {
             printf("Input: \"%s\" -> Warning: Number out of 'int' range, but fits in 'long': %ld\n", s, number);
        } else {
             printf("Input: \"%s\" -> Converted to integer: %ld\n", s, number);
        }
    }
}

int main(int argc, char* argv[]) {
    // Handling command-line arguments if provided
    // argv[0] is the program name, arguments start at index 1
    if (argc > 1) {
        printf("--- Processing Command-Line Arguments ---\n");
        for (int i = 1; i < argc; ++i) {
            convertStringToInt(argv[i]);
        }
        printf("---------------------------------------\n");
    }

    // 5 hardcoded test cases
    printf("\n--- Hardcoded Test Cases ---\n");
    const char* testCases[] = {"123", "-456", "abc", "9999999999999999999", "789xyz"};
    int numTests = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < numTests; ++i) {
        convertStringToInt(testCases[i]);
    }
    printf("--------------------------\n");

    return 0;
}