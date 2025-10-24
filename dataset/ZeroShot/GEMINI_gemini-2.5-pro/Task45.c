#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // For errno
#include <math.h>  // For HUGE_VAL and isfinite

// Define error codes for clear error reporting
#define SUCCESS 0
#define ERROR_NULL_INPUT -1
#define ERROR_EMPTY_INPUT -2
#define ERROR_INVALID_FORMAT -3
#define ERROR_DIVIDE_BY_ZERO -4
#define ERROR_OUT_OF_RANGE -5

/**
 * Securely converts two string inputs to doubles and performs division.
 * It handles null/empty inputs, non-numeric formats, and division by zero.
 *
 * @param numStr1 The string representing the numerator.
 * @param numStr2 The string representing the denominator.
 * @param result Pointer to a double where the result will be stored.
 * @return An integer error code (0 for success).
 */
int performDivision(const char* numStr1, const char* numStr2, double* result) {
    // 1. Validate for null inputs
    if (numStr1 == NULL || numStr2 == NULL || result == NULL) {
        return ERROR_NULL_INPUT;
    }
    
    // 2. Validate for empty inputs
    if (*numStr1 == '\0' || *numStr2 == '\0') {
        return ERROR_EMPTY_INPUT;
    }

    char* endptr1;
    char* endptr2;
    double numerator, denominator;

    // 3. Convert strings to numbers, handling format and range errors
    errno = 0; // Reset errno before the call
    numerator = strtod(numStr1, &endptr1);
    // Check for conversion errors
    if (errno == ERANGE) {
        return ERROR_OUT_OF_RANGE;
    }
    // Check if the whole string was a valid number
    if (endptr1 == numStr1 || *endptr1 != '\0') {
        return ERROR_INVALID_FORMAT;
    }

    errno = 0; // Reset for the second conversion
    denominator = strtod(numStr2, &endptr2);
    if (errno == ERANGE) {
        return ERROR_OUT_OF_RANGE;
    }
    if (endptr2 == numStr2 || *endptr2 != '\0') {
        return ERROR_INVALID_FORMAT;
    }

    // 4. Check for division by zero
    if (denominator == 0.0) {
        return ERROR_DIVIDE_BY_ZERO;
    }

    *result = numerator / denominator;
    
    // Check if the result of the division is valid (not infinity)
    if (!isfinite(*result)) {
        return ERROR_OUT_OF_RANGE;
    }

    return SUCCESS;
}

// Helper function to print error messages based on code
void printError(int errorCode) {
    switch (errorCode) {
        case ERROR_NULL_INPUT:
            fprintf(stderr, "Error: One or more input pointers are NULL.\n");
            break;
        case ERROR_EMPTY_INPUT:
            fprintf(stderr, "Error: Input strings cannot be empty.\n");
            break;
        case ERROR_INVALID_FORMAT:
            fprintf(stderr, "Error: Invalid number format provided.\n");
            break;
        case ERROR_DIVIDE_BY_ZERO:
            fprintf(stderr, "Error: Cannot divide by zero.\n");
            break;
        case ERROR_OUT_OF_RANGE:
            fprintf(stderr, "Error: Number is out of valid range.\n");
            break;
        default:
            fprintf(stderr, "An unknown error occurred.\n");
    }
}

int main() {
    // --- Test Cases ---
    const char* test_inputs[][2] = {
        {"100", "20"},       // Test Case 1: Valid input
        {"10", "0"},         // Test Case 2: Division by zero
        {"abc", "5"},        // Test Case 3: Invalid number format
        {"50", NULL},        // Test Case 4: Null input
        {"", "10"}           // Test Case 5: Empty string input
    };
    
    int num_tests = sizeof(test_inputs) / sizeof(test_inputs[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Test Case %d: Valid input ('%s', '%s')\n", 
               i + 1, 
               test_inputs[i][0] ? test_inputs[i][0] : "NULL", 
               test_inputs[i][1] ? test_inputs[i][1] : "NULL");

        double result;
        int status = performDivision(test_inputs[i][0], test_inputs[i][1], &result);

        if (status == SUCCESS) {
            printf("Result: %f\n", result);
        } else {
            printError(status);
        }
        printf("--------------------\n");
    }

    return 0;
}