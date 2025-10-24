#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * Attempts to convert a string to an integer.
 * Handles invalid formats and values outside the range of a standard int.
 *
 * @param inputStr The null-terminated string to be converted.
 * @param outValue A pointer to an integer where the result will be stored.
 * @return 1 if conversion is successful, 0 otherwise.
 */
int convertStringToInt(const char* inputStr, int* outValue) {
    if (inputStr == NULL || *inputStr == '\0') {
        return 0; // Null or empty string is invalid
    }
    
    char* endptr;
    long long_val;

    // Set errno to 0 before the call to distinguish our own errors
    errno = 0;
    
    // Use strtol for robust conversion. It handles whitespace and signs.
    long_val = strtol(inputStr, &endptr, 10);
    
    // Check for various error conditions.
    // 1. If strtol had a range error (ERANGE).
    if (errno == ERANGE) {
        return 0;
    }
    
    // 2. If no digits were found at all.
    if (endptr == inputStr) {
        return 0;
    }
    
    // 3. If there are trailing characters after the number that are not whitespace.
    //    We check the entire rest of the string for non-whitespace.
    char* p = endptr;
    while (*p != '\0') {
        if (!(*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')) {
            return 0; // Invalid trailing characters
        }
        p++;
    }
    
    // 4. If the value is outside the range of an int.
    if (long_val < INT_MIN || long_val > INT_MAX) {
        return 0;
    }
    
    // If all checks pass, the conversion is successful.
    *outValue = (int)long_val;
    return 1;
}

int main(int argc, char* argv[]) {
    // Section 1: Process command-line argument if provided
    if (argc > 1) {
        printf("--- Processing Command-Line Argument ---\n");
        const char* cliInput = argv[1];
        printf("Input: \"%s\"\n", cliInput);
        int result;
        if (convertStringToInt(cliInput, &result)) {
            printf("Successfully converted to: %d\n", result);
        } else {
            printf("Error: Conversion failed. Input is not a valid integer.\n");
        }
        printf("--------------------------------------\n\n");
    } else {
        printf("--- No Command-Line Argument Provided ---\n\n");
    }

    // Section 2: Run 5 hardcoded test cases
    printf("--- Running Hardcoded Test Cases ---\n");
    const char* testCases[] = {
        "999",          // Valid integer
        "-42",          // Valid negative integer
        "50frogs",      // Invalid format (trailing chars)
        "2147483647",   // Valid (INT_MAX)
        "-2147483649"   // Out of range (INT_MIN - 1)
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTestCases; ++i) {
        const char* testInput = testCases[i];
        printf("Test Case %d:\n", i + 1);
        printf("Input: \"%s\"\n", testInput);
        int result;
        if (convertStringToInt(testInput, &result)) {
            printf("Successfully converted to: %d\n", result);
        } else {
            printf("Error: Conversion failed. Input is not a valid integer.\n");
        }
        printf("\n"); // For spacing
    }
    printf("------------------------------------\n");

    return 0;
}