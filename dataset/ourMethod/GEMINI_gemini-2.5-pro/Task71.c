#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Safely converts a string to an integer.
 * 
 * This function uses strtol for safe conversion, checking for various error conditions
 * including no valid digits, trailing characters, and range overflows.
 * 
 * @param s The null-terminated string to convert.
 * @param out_val A pointer to an integer where the result will be stored on success.
 * @return 0 on success, 1 on failure.
 */
int convertStringToInt(const char* s, int* out_val) {
    char* endptr;
    long val;

    if (s == NULL || *s == '\0') {
        return 1; // Failure on null or empty string
    }

    errno = 0; // Reset errno before the call
    val = strtol(s, &endptr, 10);

    // Check for various error conditions:
    // 1. No digits were found at all.
    if (endptr == s) {
        return 1;
    }
    
    // 2. The entire string was not consumed, meaning there are trailing characters.
    if (*endptr != '\0') {
        return 1;
    }

    // 3. Overflow or underflow occurred.
    if (errno == ERANGE || val > INT_MAX || val < INT_MIN) {
        return 1;
    }

    *out_val = (int)val;
    return 0; // Success
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        printf("Processing command line argument:\n");
        int result;
        if (convertStringToInt(argv[1], &result) == 0) {
            printf("Successfully converted \"%s\" to %d\n", argv[1], result);
        } else {
            printf("Error: Could not convert \"%s\" to an integer.\n", argv[1]);
        }
    } else {
        printf("No command line arguments provided. Running test cases:\n");
        const char* test_cases[] = {
            "123",          // Valid positive integer
            "-456",         // Valid negative integer
            "0",            // Valid zero
            "abc",          // Invalid format
            "2147483648"    // Out of range for a 32-bit int
        };
        int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

        for (int i = 0; i < num_tests; ++i) {
            int result;
            if (convertStringToInt(test_cases[i], &result) == 0) {
                printf("Input: \"%s\", Output: %d\n", test_cases[i], result);
            } else {
                printf("Input: \"%s\", Error: Invalid number format or out of range.\n", test_cases[i]);
            }
        }
    }
    return 0;
}