#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Define status codes for error handling
#define SUCCESS 0
#define ERROR_INVALID_FORMAT 1
#define ERROR_DIVIDE_BY_ZERO 2
#define ERROR_OUT_OF_RANGE 3

/**
 * @brief Performs division on two numbers provided as strings.
 *
 * This function uses return codes and an output parameter for error handling,
 * which is idiomatic in C.
 *
 * @param numeratorStr The string representation of the numerator.
 * @param denominatorStr The string representation of the denominator.
 * @param result A pointer to a double where the result will be stored on success.
 * @return An integer status code (SUCCESS, ERROR_INVALID_FORMAT, etc.).
 */
int performDivision(const char* numeratorStr, const char* denominatorStr, double* result) {
    char* endptr_num;
    char* endptr_den;
    double numerator, denominator;

    // Reset errno before the call
    errno = 0;

    // Convert numerator string to double
    numerator = strtod(numeratorStr, &endptr_num);
    // Check for conversion errors
    if (errno == ERANGE) return ERROR_OUT_OF_RANGE;
    if (endptr_num == numeratorStr || *endptr_num != '\0') {
        return ERROR_INVALID_FORMAT; // No conversion or trailing characters
    }

    // Reset errno before the second call
    errno = 0;
    
    // Convert denominator string to double
    denominator = strtod(denominatorStr, &endptr_den);
    // Check for conversion errors
    if (errno == ERANGE) return ERROR_OUT_OF_RANGE;
    if (endptr_den == denominatorStr || *endptr_den != '\0') {
        return ERROR_INVALID_FORMAT; // No conversion or trailing characters
    }

    // Check for division by zero
    if (denominator == 0.0) {
        return ERROR_DIVIDE_BY_ZERO;
    }

    // Perform calculation and store in the output parameter
    *result = numerator / denominator;

    return SUCCESS;
}

int main() {
    printf("--- C Test Cases ---\n");
    
    const char* test_cases[5][2] = {
        {"10", "2"},
        {"5", "0"},
        {"abc", "5"},
        {"10", "xyz"},
        {"7.5", "1.5"}
    };

    for (int i = 0; i < 5; ++i) {
        double result_val;
        printf("Test %d ('%s', '%s'): ", i + 1, test_cases[i][0], test_cases[i][1]);
        
        int status = performDivision(test_cases[i][0], test_cases[i][1], &result_val);

        switch (status) {
            case SUCCESS:
                printf("Result: %f\n", result_val);
                break;
            case ERROR_INVALID_FORMAT:
                printf("Error: Invalid number format. Please provide valid numbers.\n");
                break;
            case ERROR_DIVIDE_BY_ZERO:
                printf("Error: Cannot divide by zero.\n");
                break;
            case ERROR_OUT_OF_RANGE:
                 printf("Error: Number is out of range.\n");
                 break;
            default:
                printf("An unexpected error occurred.\n");
                break;
        }
    }

    return 0;
}