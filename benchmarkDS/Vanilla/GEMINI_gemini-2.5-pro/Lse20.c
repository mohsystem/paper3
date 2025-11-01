#include <stdio.h>
#include <stdlib.h> // For strtol()
#include <errno.h>  // For errno

const int OFFSET = 1000;

/**
 * Adds a constant offset to the given integer value.
 * @param value The integer input.
 * @return The value with the offset added.
 */
int addOffset(int value) {
    return value + OFFSET;
}

void runTests() {
    printf("--- Running 5 Test Cases ---\n");
    int test_cases[] = {10, 0, -50, 2000, -1000};
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    for (int i = 0; i < num_tests; ++i) {
        printf("Input: %d, Output: %d\n", test_cases[i], addOffset(test_cases[i]));
    }
    printf("--------------------------\n\n");
}

int main(int argc, char *argv[]) {
    runTests();

    // Handling command line input
    if (argc > 1) {
        char *endptr;
        errno = 0; // To distinguish success/failure after call
        long inputValue = strtol(argv[1], &endptr, 10);

        // Check for conversion errors
        if (errno != 0 || endptr == argv[1] || *endptr != '\0') {
            fprintf(stderr, "Error: The provided command line argument is not a valid integer.\n");
        } else {
            int result = addOffset((int)inputValue);
            printf("Command line input: %ld\n", inputValue);
            printf("Result with offset: %d\n", result);
        }
    } else {
        printf("Usage: %s <integer_value>\n", argv[0]);
    }

    return 0;
}