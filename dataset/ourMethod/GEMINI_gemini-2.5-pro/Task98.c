#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

/**
 * Attempts to convert a string to an integer and prints the result or an error message.
 * This function uses strtol for safe conversion, which is superior to atoi.
 * It checks for:
 * 1. Null or empty input.
 * 2. Range errors (overflow/underflow) by checking errno for ERANGE.
 * 3. Invalid format (e.g., "abc") by checking if any conversion was done.
 * 4. Trailing non-whitespace characters (e.g., "123a").
 * 5. Whether the converted 'long' value fits within the 'int' range.
 *
 * @param input The character string to be converted.
 */
void convertStringToInteger(const char* input) {
    if (input == NULL) {
        printf("Input: (null) -> Error: Input is null.\n");
        return;
    }

    char* endptr;
    errno = 0; // To distinguish success/failure after the call

    long val = strtol(input, &endptr, 10);

    // Check if any digits were read
    if (input == endptr) {
        printf("Input: \"%s\" -> Error: Not a valid integer (no digits found).\n", input);
        return;
    }

    // Check for overflow or underflow
    if (errno == ERANGE) {
        printf("Input: \"%s\" -> Error: Number is out of range for a long.\n", input);
        return;
    }
    
    // Check for trailing non-whitespace characters
    char* temp = endptr;
    while (isspace((unsigned char)*temp)) {
        temp++;
    }

    if (*temp != '\0') {
        printf("Input: \"%s\" -> Error: Not a valid integer (trailing characters).\n", input);
        return;
    }

    // Check if the value fits in an int
    if (val < INT_MIN || val > INT_MAX) {
        printf("Input: \"%s\" -> Error: Number is out of range for an int.\n", input);
        return;
    }

    printf("Input: \"%s\" -> Success: %d\n", input, (int)val);
}

/**
 * Main function to run test cases for the string-to-integer conversion.
 */
int main() {
    const char* testCases[] = {
        "  123  ",          // Valid integer with whitespace
        "-456",             // Valid negative integer
        "1a2b",             // Invalid trailing characters
        "2147483648",       // Out of int range (LONG_MAX on 32-bit, but fine for long on 64-bit)
        "not a number"      // Invalid characters
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- Running 5 Test Cases ---\n");
    for (int i = 0; i < num_tests; ++i) {
        convertStringToInteger(testCases[i]);
    }
    printf("--- Test Cases Finished ---\n");

    return 0;
}