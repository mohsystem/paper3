#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define OFFSET 1000LL

/**
 * @brief Adds a constant offset to a value.
 * @param value The input number.
 * @param result A pointer to a long long where the result will be stored.
 * @return 0 on success, -1 on overflow.
 */
int addOffset(long long value, long long *result) {
    // Secure: Check for potential overflow before performing the addition.
    if (value > LLONG_MAX - OFFSET) {
        return -1; // Overflow
    }
    *result = value + OFFSET;
    return 0; // Success
}

/**
 * @brief Helper function to parse, run logic, and print results for a test case.
 * @param testStr The string input to test.
 */
void runTest(const char* testStr) {
    printf("Test Case: \"%s\" -> ", testStr);
    
    char *endptr;
    errno = 0; // Reset errno before the call

    long long inputValue = strtoll(testStr, &endptr, 10);

    // Secure: Check for various conversion errors from strtoll.
    if (endptr == testStr) {
        printf("Error: No digits were found.\n");
        return;
    }
    if (*endptr != '\0') {
        printf("Error: Trailing non-numeric characters found.\n");
        return;
    }
    if (errno == ERANGE) {
        printf("Error: Number is out of range for a long long.\n");
        return;
    }

    long long result;
    if (addOffset(inputValue, &result) != 0) {
        printf("Error: Overflow detected on addition.\n");
    } else {
        printf("Result: %lld\n", result);
    }
}

int main(int argc, char* argv[]) {
    // Part 1: Handle actual command line input
    if (argc == 2) {
        printf("--- Processing Command Line Input ---\n");
        runTest(argv[1]);
        printf("-------------------------------------\n\n");
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s <single integer value>\n", argv[0]);
    }

    // Part 2: Run 5+ built-in test cases as requested
    printf("--- Running Built-in Test Cases ---\n");
    const char* testCases[] = {
        "123",
        "0",
        "-50",
        "9223372036854775807",      // LLONG_MAX, will cause overflow
        "abc",                      // Invalid format
        "123xyz",                   // Trailing characters
        "999999999999999999999999"  // Out of range for strtoll
    };
    int numTests = sizeof(testCases) / sizeof(testCases[0]);
    for (int i = 0; i < numTests; ++i) {
        runTest(testCases[i]);
    }
    printf("-----------------------------------\n");

    return 0;
}