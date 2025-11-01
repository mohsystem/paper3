#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Parses a string to a long, adds 1000, and stores the result.
 * This function is secure against invalid number formats and overflow.
 * @param inputStr The string representation of the number.
 * @param result Pointer to a long where the result will be stored.
 * @return 0 on success, -1 on failure. Errors are printed to stderr.
 */
int addThousand(const char *inputStr, long *result) {
    if (inputStr == NULL || result == NULL) {
        fprintf(stderr, "Error: Null pointer provided to function.\n");
        return -1;
    }

    char *endptr;
    errno = 0; // Secure: Reset errno before the call to strtol.

    long num = strtol(inputStr, &endptr, 10);

    // Secure: Check for various conversion errors.
    if (endptr == inputStr) {
        fprintf(stderr, "Error: No digits were found in input.\n");
        return -1;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Trailing non-numeric characters found: %s\n", endptr);
        return -1;
    }
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Input number is out of range for a long integer.\n");
        return -1;
    }

    // Secure: Check for potential overflow before performing the addition.
    if (num > LONG_MAX - 1000) {
        fprintf(stderr, "Error: Addition would cause an overflow.\n");
        return -1;
    }

    *result = num + 1000;
    return 0;
}

void runTestCase(int caseNum, const char* input) {
    printf("Test Case %d: Input = \"%s\"\n", caseNum, input);
    long result;
    if (addThousand(input, &result) == 0) {
        printf("  -> Result: %ld\n", result);
    } else {
        // Error message is printed inside the addThousand function.
        printf("  -> Operation failed.\n");
    }
}

int main(int argc, char *argv[]) {
    // Part 1: Process command-line argument if provided.
    if (argc == 2) {
        printf("Processing command-line argument: %s\n", argv[1]);
        long result;
        if (addThousand(argv[1], &result) == 0) {
            printf("Result: %ld\n", result);
        } else {
            return EXIT_FAILURE; // Exit with an error code on failure.
        }
        printf("\n-----------------------------------------\n\n");
    } else if (argc > 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        fprintf(stderr, "Ignoring extra arguments and running test cases.\n");
        printf("\n-----------------------------------------\n\n");
    }

    // Part 2: Run 5 hardcoded test cases to demonstrate functionality.
    printf("Running 5 test cases...\n");
    runTestCase(1, "50");
    runTestCase(2, "-50");
    runTestCase(3, "0");
    runTestCase(4, "abc");
    
    // Test case 5: A number that will cause an overflow.
    char long_max_str[30];
    sprintf(long_max_str, "%ld", LONG_MAX);
    runTestCase(5, long_max_str);

    return EXIT_SUCCESS;
}