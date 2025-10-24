#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

/**
 * Attempts to convert a given string to an integer and prints the outcome.
 * @param input The string to be converted.
 */
void attemptConversion(const char* input) {
    char* endptr;
    errno = 0; // To distinguish between 0 as a result and an error

    long number = strtol(input, &endptr, 10);

    // Check for various error conditions:
    // 1. If endptr is same as input, no digits were found.
    if (endptr == input) {
        printf("Input: \"%s\" -> Failed. Not a valid integer.\n", input);
        return;
    }

    // 2. If errno is ERANGE, the number was out of range for a long.
    if (errno == ERANGE) {
        printf("Input: \"%s\" -> Failed. Number is out of range.\n", input);
        return;
    }

    // 3. Check for trailing non-whitespace characters.
    //    strtol stops at the first invalid character (e.g., 'a' in "12a34").
    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }

    // If we are not at the end of the string, it's an invalid format.
    if (*endptr != '\0') {
        printf("Input: \"%s\" -> Failed. Not a valid integer.\n", input);
        return;
    }

    // If all checks pass, the conversion was successful.
    printf("Input: \"%s\" -> Success! Converted to integer: %ld\n", input, number);
}

int main() {
    // Array of 5 test cases to demonstrate the functionality.
    const char* testCases[] = {
        "123",      // Valid positive integer
        "-45",      // Valid negative integer
        "  99  ",   // Valid integer with whitespace
        "abc",      // Invalid string
        "12a34"     // Invalid string with mixed characters
    };
    int numTests = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- C: Testing Integer Conversion ---\n");
    for (int i = 0; i < numTests; i++) {
        attemptConversion(testCases[i]);
    }

    return 0;
}