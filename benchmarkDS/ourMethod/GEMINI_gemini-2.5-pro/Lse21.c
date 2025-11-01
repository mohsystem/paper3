#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Parses a string to a long long, adds 1000, and checks for overflow.
 * @param inputStr The input string to parse.
 * @param result Pointer to a long long to store the result.
 * @return 0 on success, 1 on parsing error, 2 on overflow/range error.
 */
int processValue(const char *inputStr, long long *result) {
    if (inputStr == NULL || result == NULL) {
        return 1; // Invalid argument
    }

    char *endptr;
    errno = 0; // To distinguish success/failure after call

    long long value = strtoll(inputStr, &endptr, 10);

    // Check for conversion errors based on errno and endptr
    if (errno == ERANGE) {
        return 2; // Value was out of range for long long
    }
    if (endptr == inputStr) {
        return 1; // No digits were found
    }
    if (*endptr != '\0') {
        return 1; // Trailing non-numeric characters
    }

    // Check for overflow before adding 1000
    if (value > LLONG_MAX - 1000) {
        return 2;
    }

    *result = value + 1000;
    return 0; // Success
}

/**
 * @brief Runs a set of test cases against the processValue function.
 */
void runTests() {
    printf("--- Running Test Cases ---\n");
    
    char max_ll_str[30];
    snprintf(max_ll_str, sizeof(max_ll_str), "%lld", LLONG_MAX);
    
    const char* test_cases[] = {
        "10",                     // Valid positive
        "0",                      // Valid zero
        "-50",                    // Valid negative
        "not a number",           // Invalid format
        max_ll_str                // Overflow on add
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* test = test_cases[i];
        printf("Input: \"%s\" -> ", test);
        long long result;
        int ret = processValue(test, &result);
        if (ret == 0) {
            printf("Output: %lld\n", result);
        } else if (ret == 1) {
            printf("Error: Invalid number format.\n");
        } else if (ret == 2) {
            printf("Error: Number out of range or overflow.\n");
        }
    }
}

/**
 * @brief Main function.
 * If one argument is provided, it's processed.
 * If no arguments, tests are run.
 * Otherwise, usage is shown.
 */
int main(int argc, char *argv[]) {
    if (argc == 2) {
        long long result;
        int ret = processValue(argv[1], &result);

        if (ret == 0) {
            printf("%lld\n", result);
            return EXIT_SUCCESS;
        } else {
            if (ret == 1) {
                fprintf(stderr, "Error: Invalid number format provided.\n");
            } else if (ret == 2) {
                fprintf(stderr, "Error: Input value is out of range or would cause overflow.\n");
            }
            return EXIT_FAILURE;
        }
    } else if (argc == 1) {
        runTests();
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Usage: %s <integer>\n", argv[0]);
        return EXIT_FAILURE;
    }
}