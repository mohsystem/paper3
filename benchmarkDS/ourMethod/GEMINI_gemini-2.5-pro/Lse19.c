#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * Parses a string to a long long, adds 1000 to it, and stores the result.
 * This function is secure against non-numeric input and integer overflow.
 *
 * @param inputStr The string to parse.
 * @param result Pointer to a long long where the result will be stored.
 * @return 0 on success, -1 on failure.
 */
int addThousand(const char* inputStr, long long* result) {
    if (inputStr == NULL || result == NULL) {
        fprintf(stderr, "Error: Null pointer provided.\n");
        return -1;
    }

    char* endptr;
    errno = 0; // Reset errno before the call

    long long value = strtoll(inputStr, &endptr, 10);

    // Check for conversion errors
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Input '%s' is out of range for a long long.\n", inputStr);
        return -1;
    }
    if (endptr == inputStr) {
        fprintf(stderr, "Error: No digits were found in '%s'.\n", inputStr);
        return -1;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Trailing non-numeric characters found in '%s'.\n", inputStr);
        return -1;
    }

    // Check for potential overflow before adding
    if (value > LLONG_MAX - 1000) {
        fprintf(stderr, "Error: Input value %lld is too large and would cause an overflow.\n", value);
        return -1;
    }

    *result = value + 1000;
    return 0; // Success
}

/**
 * Runs a series of test cases for the addThousand function.
 */
void runTests() {
    printf("\n--- Running Test Cases ---\n");
    const char* testCases[] = {
        "10",
        "0",
        "-50",
        "9223372036854775807", // LLONG_MAX, will cause overflow
        "not a number",
        "123 with text",
        "9223372036854774807" // LLONG_MAX - 1000, valid
    };
    int numTests = sizeof(testCases) / sizeof(testCases[0]);

    for (int i = 0; i < numTests; ++i) {
        printf("Test case: \"%s\" -> ", testCases[i]);
        long long res;
        if (addThousand(testCases[i], &res) == 0) {
            printf("Result: %lld\n", res);
        } else {
            printf("Handled error as expected.\n");
        }
    }
    printf("--- Test Cases Finished ---\n\n");
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        printf("Processing command line argument: %s\n", argv[1]);
        long long result;
        if (addThousand(argv[1], &result) == 0) {
            printf("Result: %lld\n", result);
        } else {
            fprintf(stderr, "Failed to process command line argument.\n");
            return EXIT_FAILURE;
        }
    } else {
        printf("Usage: %s <number>\n", argv[0]);
        printf("No command line argument provided. Running test cases instead.\n");
    }
    
    // Running 5+ test cases as requested.
    runTests();
    
    return EXIT_SUCCESS;
}