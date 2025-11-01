#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Adds 1000 to the given value.
 * @param value The input number (long).
 * @return The input number plus 1000.
 */
long addThousand(long value) {
    return value + 1000;
}

/**
 * @brief Runs a set of predefined test cases to verify the addThousand logic.
 */
void runTestCases() {
    printf("--- Running 5 Test Cases ---\n");
    long testInputs[] = {50, 0, -50, LONG_MAX - 1000, LONG_MIN};
    long expectedOutputs[] = {1050, 1000, 950, LONG_MAX, LONG_MIN + 1000};
    int num_tests = sizeof(testInputs) / sizeof(testInputs[0]);

    for (int i = 0; i < num_tests; ++i) {
        long result = addThousand(testInputs[i]);
        printf("Test %d: Input=%ld, Output=%ld, Expected=%ld, Pass=%s\n",
               i + 1, testInputs[i], result, expectedOutputs[i],
               result == expectedOutputs[i] ? "true" : "false");
    }
    printf("--- Test Cases Finished ---\n\n");
}

/**
 * @brief Main function. Runs test cases and processes a command-line argument.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Always returns 0.
 */
int main(int argc, char* argv[]) {
    // Run test cases to demonstrate functionality
    runTestCases();
    
    printf("--- Processing Command-Line Argument ---\n");
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number>\n", argv[0]);
        fprintf(stderr, "Please provide exactly one numeric argument.\n");
        return 0; // As per prompt, return 0 even on usage error
    }

    char *endptr;
    errno = 0; // To distinguish success/failure after call
    long inputValue = strtol(argv[1], &endptr, 10);

    // Check for conversion errors.
    if ((errno == ERANGE && (inputValue == LONG_MAX || inputValue == LONG_MIN)) || (errno != 0 && inputValue == 0)) {
        fprintf(stderr, "Error: The provided argument '%s' is out of range for a long integer.\n", argv[1]);
    } else if (endptr == argv[1]) {
        fprintf(stderr, "Error: No digits were found in the argument '%s'.\n", argv[1]);
    } else if (*endptr != '\0') {
        fprintf(stderr, "Error: The argument '%s' contains non-numeric characters.\n", argv[1]);
    } else {
        // If we reach here, input is a valid long
        long result = addThousand(inputValue);
        printf("Result from command-line input: %ld\n", result);
    }
    
    return 0;
}