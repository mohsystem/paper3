#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>

// A struct to hold the result of the conversion, indicating success or failure.
typedef struct {
    int value;
    bool success;
} ConversionResult;

/**
 * Securely converts a string to an integer using strtol.
 * This is safer than atoi() because it provides error checking.
 *
 * @param input The null-terminated string to convert.
 * @return A ConversionResult struct. The 'success' field is true if
 *         conversion was successful, and false otherwise. If successful,
 *         the 'value' field holds the converted integer.
 */
ConversionResult convertStringToInt(const char* input) {
    ConversionResult res = {0, false};
    char* endptr;

    if (input == NULL || *input == '\0' || strspn(input, " \t\n\v\f\r") == strlen(input)) {
        fprintf(stderr, "Error: Input string is null, empty, or contains only whitespace.\n");
        return res;
    }

    // Reset errno before the call to distinguish new errors from old ones.
    errno = 0;
    long converted_value = strtol(input, &endptr, 10);

    // Check for various error conditions.
    // 1. Check if an overflow/underflow error occurred during conversion.
    if (errno == ERANGE) {
        fprintf(stderr, "Error: '%s' is out of range for a long integer.\n", input);
        return res;
    }

    // 2. Check if the string was not a number at all.
    if (endptr == input) {
        fprintf(stderr, "Error: '%s' is not a valid number.\n", input);
        return res;
    }

    // 3. Check for trailing characters (e.g., "123abc").
    // We allow trailing whitespace.
    while (*endptr != '\0' && (*endptr == ' ' || *endptr == '\t')) {
        endptr++;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Input string '%s' contains trailing non-numeric characters.\n", input);
        return res;
    }

    // 4. Check if the converted long value is within the range of an int.
    if (converted_value < INT_MIN || converted_value > INT_MAX) {
        fprintf(stderr, "Error: '%s' is out of the range for a standard integer.\n", input);
        return res;
    }

    // If we get here, the conversion was successful.
    res.value = (int)converted_value;
    res.success = true;
    return res;
}

int main(int argc, char* argv[]) {
    // Handle command-line argument if provided
    if (argc > 1) {
        printf("--- Command-Line Input ---\n");
        const char* cliInput = argv[1];
        printf("Input: \"%s\"\n", cliInput);
        ConversionResult result = convertStringToInt(cliInput);
        if (result.success) {
            printf("Successfully converted to: %d\n", result.value);
        } else {
            printf("Conversion failed.\n");
        }
        printf("--------------------------\n\n");
    } else {
        printf("Usage: ./<executable_name> <string_to_convert>\n");
        printf("Running built-in test cases as no command-line argument was provided.\n\n");
    }

    // Run 5 test cases
    printf("--- Built-in Test Cases ---\n");
    const char* testCases[] = {
        "123",          // 1. Valid positive integer
        "-456",         // 2. Valid negative integer
        "hello",        // 3. Invalid format
        "2147483648",   // 4. Integer overflow
        "-2147483649"   // 5. Integer underflow
    };
    int num_tests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Input: \"%s\"\n", testCases[i]);
        ConversionResult result = convertStringToInt(testCases[i]);
        if (result.success) {
            printf("Successfully converted to: %d\n", result.value);
        } else {
            printf("Conversion failed.\n");
        }
        printf("\n"); // for spacing
    }
    printf("---------------------------\n");

    return 0;
}