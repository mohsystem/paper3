#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * Attempts to convert a string to an integer and prints the result or an error message.
 *
 * @param input The string to be converted.
 */
void convertAndPrint(const char* input) {
    if (input == NULL || *input == '\0') {
        printf("Error: Invalid input. String is null or empty.\n");
        return;
    }

    char* endptr;
    errno = 0; // To distinguish between 0 and an error

    long result = strtol(input, &endptr, 10);

    // Check for conversion errors
    if (endptr == input) {
        printf("Error: Invalid input. No conversion could be performed for '%s'.\n", input);
    } else if (*endptr != '\0') {
        printf("Error: Invalid input. Trailing characters found in '%s'.\n", input);
    } else if (errno == ERANGE) {
        printf("Error: Out of range. The value '%s' is too large or too small.\n", input);
    } else if (result > INT_MAX || result < INT_MIN) {
        printf("Error: Out of range. The value '%s' does not fit in a standard integer.\n", input);
    } else {
        int number = (int)result;
        printf("Successfully converted to integer: %d\n", number);
    }
}

int main() {
    const char* testCases[] = {"123", "-456", "abc", "2147483648", "123a"};
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- Running C Test Cases ---\n");
    for (int i = 0; i < numTestCases; ++i) {
        printf("Input: \"%s\" -> ", testCases[i]);
        convertAndPrint(testCases[i]);
    }
    printf("--- End of Test Cases ---\n");
    return 0;
}