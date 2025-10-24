#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>   // For errno and ERANGE
#include <limits.h>  // For INT_MIN and INT_MAX
#include <stdbool.h> // For bool type
#include <ctype.h>   // For isspace

/**
 * Attempts to convert a string to an integer securely.
 * It uses strtol instead of atoi for robust error checking. It checks for:
 * 1. Null input.
 * 2. No valid digits in the string.
 * 3. Trailing non-whitespace characters after the number.
 * 4. Overflow or underflow conditions.
 *
 * @param input The null-terminated string to convert.
 * @param result A pointer to an integer where the result will be stored.
 * @return true if the conversion was successful, false otherwise.
 */
bool convertToInteger(const char* input, int* result) {
    if (input == NULL || result == NULL) {
        return false;
    }

    char* endptr;
    long val;

    // Reset errno before the call.
    errno = 0;

    val = strtol(input, &endptr, 10);

    // Check for various error conditions.
    // 1. If strtol's parsing failed (e.g., empty string or only whitespace).
    if (endptr == input) {
        return false;
    }

    // 2. If an overflow or underflow occurred.
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        return false;
    }

    // 3. Check for trailing non-whitespace characters.
    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }
    if (*endptr != '\0') {
        return false;
    }

    *result = (int)val;
    return true;
}

int main() {
    const char* testCases[] = {
        "123",        // Valid positive integer
        "-456",       // Valid negative integer
        "  789  ",    // Valid integer with whitespace
        "abc",        // Invalid non-numeric string
        "2147483648", // Out of range for a 32-bit signed integer
        "123a",       // Trailing non-numeric characters
        ""            // Empty string
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("Running C Test Cases:\n");
    for (int i = 0; i < numTestCases; ++i) {
        printf("Input: \"%s\" -> ", testCases[i]);
        int value;
        if (convertToInteger(testCases[i], &value)) {
            printf("Success, value: %d\n", value);
        } else {
            printf("Failed to convert.\n");
        }
    }

    return 0;
}