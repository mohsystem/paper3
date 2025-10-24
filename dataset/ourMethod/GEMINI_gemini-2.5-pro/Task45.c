#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

// Enumeration for clear error codes.
typedef enum {
    SUCCESS = 0,
    ERROR_INVALID_INPUT,
    ERROR_OUT_OF_RANGE,
    ERROR_DIVISION_BY_ZERO
} StatusCode;

// Converts a string to an integer with robust error checking.
// Rule #1: Ensure all input is validated.
// Rule #6: Treat all inputs as untrusted.
StatusCode stringToInteger(const char* inputStr, int* result) {
    char* endptr;
    long val;

    // C Language Security Checklist: Initialize pointers.
    if (result == NULL || inputStr == NULL) {
        return ERROR_INVALID_INPUT;
    }
    
    // C Language Security Checklist: All pointers initialized to NULL or valid memory.
    endptr = NULL;
    
    // C Language Security Checklist: Check function return values (errno).
    errno = 0; // Reset errno before the call to strtol.
    val = strtol(inputStr, &endptr, 10);

    // Check for various error conditions from strtol.
    // Rule #1: Validate ranges, lengths, and types.
    if (errno == ERANGE) {
        return ERROR_OUT_OF_RANGE;
    }
    if (endptr == inputStr) {
        // No digits were found in the string.
        return ERROR_INVALID_INPUT;
    }
    // Check for trailing non-whitespace characters.
    while (*endptr != '\0') {
        if (!(*endptr == ' ' || *endptr == '\t' || *endptr == '\n' || *endptr == '\r')) {
            return ERROR_INVALID_INPUT;
        }
        endptr++;
    }

    // C Language Security Checklist: Integer overflow checks.
    if (val < INT_MIN || val > INT_MAX) {
        return ERROR_OUT_OF_RANGE;
    }

    *result = (int)val;
    return SUCCESS;
}

// Performs division, checking for division by zero.
// Rule #6: Keep functions small and testable.
StatusCode performDivision(int numerator, int denominator, double* result) {
    if (result == NULL) {
        return ERROR_INVALID_INPUT; // Should not happen with internal calls
    }
    if (denominator == 0) {
        return ERROR_DIVISION_BY_ZERO;
    }
    *result = (double)numerator / denominator;
    return SUCCESS;
}

// Encapsulates the logic for a single division operation to be called by tests.
void processDivision(const char* numStr, const char* denStr) {
    int numerator, denominator;
    double result;
    StatusCode status;

    printf("Input: (%s / %s)\n", numStr, denStr);

    // Rule #1: Validate and convert numerator.
    status = stringToInteger(numStr, &numerator);
    if (status != SUCCESS) {
        switch (status) {
            case ERROR_INVALID_INPUT:
                fprintf(stderr, "Error: Numerator is not a valid integer.\n");
                break;
            case ERROR_OUT_OF_RANGE:
                fprintf(stderr, "Error: Numerator is out of the valid integer range.\n");
                break;
            default:
                fprintf(stderr, "An unknown error occurred during numerator conversion.\n");
        }
        printf("--------------------\n");
        return;
    }
    
    // Rule #1: Validate and convert denominator.
    status = stringToInteger(denStr, &denominator);
    if (status != SUCCESS) {
        switch (status) {
            case ERROR_INVALID_INPUT:
                fprintf(stderr, "Error: Denominator is not a valid integer.\n");
                break;
            case ERROR_OUT_OF_RANGE:
                fprintf(stderr, "Error: Denominator is out of the valid integer range.\n");
                break;
            default:
                fprintf(stderr, "An unknown error occurred during denominator conversion.\n");
        }
        printf("--------------------\n");
        return;
    }
    
    // Perform the operation and handle potential errors.
    status = performDivision(numerator, denominator, &result);
    if (status == SUCCESS) {
        printf("Result: %f\n", result);
    } else if (status == ERROR_DIVISION_BY_ZERO) {
        fprintf(stderr, "Error: Division by zero is not allowed.\n");
    } else {
        fprintf(stderr, "An unknown error occurred during division.\n");
    }

    printf("--------------------\n");
}


int main() {
    // --- Test Cases ---

    // 1. Valid input
    printf("Test Case 1: Valid input\n");
    processDivision("100", "5");

    // 2. Division by zero
    printf("Test Case 2: Division by zero\n");
    processDivision("42", "0");

    // 3. Invalid numerator (non-numeric)
    printf("Test Case 3: Invalid numerator\n");
    processDivision("abc", "5");

    // 4. Invalid denominator (contains trailing non-numeric characters)
    printf("Test Case 4: Trailing characters in denominator\n");
    processDivision("100", "5x");
    
    // 5. Out of range input
    printf("Test Case 5: Out of range input\n");
    processDivision("3000000000", "2"); // Larger than max int

    return 0;
}