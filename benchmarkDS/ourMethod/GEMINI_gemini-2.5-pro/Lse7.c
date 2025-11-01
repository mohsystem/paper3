#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Retrieves an argument from an array at an index specified by another argument.
 * 
 * @param argc The number of command-line arguments.
 * @param argv An array of command-line argument strings.
 * @return 0 on success, 1 on failure. Prints the result or an error message.
 */
int getArgumentAtIndex(int argc, char* argv[]) {
    // Rule #4: Validate input - ensure there are enough arguments
    if (argc < 2) {
        fprintf(stderr, "Error: Insufficient arguments. At least two are required.\n");
        return 1;
    }

    char* endptr;
    long index;

    // Rule #5: Use a safe function for string-to-integer conversion
    errno = 0; // Per POSIX, must be set to 0 before the call
    index = strtol(argv[1], &endptr, 10);

    // Rule #4: Validate input format
    // Check if any conversion was performed
    if (endptr == argv[1]) {
        fprintf(stderr, "Error: The second argument '%s' is not a valid number.\n", argv[1]);
        return 1;
    }
    // Check if the entire string was consumed
    if (*endptr != '\0') {
        fprintf(stderr, "Error: The second argument '%s' contains non-numeric characters.\n", argv[1]);
        return 1;
    }
    // Check for overflow/underflow errors during conversion
    if (errno == ERANGE) {
        fprintf(stderr, "Error: The index value '%s' is out of range.\n", argv[1]);
        return 1;
    }

    // Rule #1, #2: Ensure the index is within the buffer's boundaries
    if (index < 0 || index >= argc) {
        fprintf(stderr, "Error: Index %ld is out of bounds for the arguments array (size: %d).\n", index, argc);
        return 1;
    }

    printf("%s\n", argv[index]);
    return 0;
}

void runTest(const char* testName, int argc, char* argv[]) {
    printf("\n--- %s ---\n", testName);
    printf("Input: ");
    for (int i = 0; i < argc; ++i) {
        printf("\"%s\" ", argv[i]);
    }
    printf("\n");
    printf("Output: ");
    getArgumentAtIndex(argc, argv);
}

int main(int argc, char* argv[]) {
    // Process command-line arguments if provided
    if (argc > 1) {
        printf("--- Processing Command Line Arguments ---\n");
        runTest("Command Line Input", argc, argv);
        printf("----------------------------------------\n");
    }

    printf("\n--- Running Built-in Test Cases ---\n");

    // Test Case 1: Valid index
    char* test1[] = {"./program", "2", "target_arg", "arg3"};
    runTest("Test Case 1: Valid index", 4, test1);

    // Test Case 2: Index pointing to the program name
    char* test2[] = {"./program", "0", "arg2"};
    runTest("Test Case 2: Index points to program name", 3, test2);

    // Test Case 3: Index out of bounds (too high)
    char* test3[] = {"./program", "3"};
    runTest("Test Case 3: Index out of bounds (high)", 2, test3);

    // Test Case 4: Invalid index format (not an integer)
    char* test4[] = {"./program", "two", "arg2"};
    runTest("Test Case 4: Invalid index format", 3, test4);

    // Test Case 5: Insufficient arguments
    char* test5[] = {"./program"};
    runTest("Test Case 5: Insufficient arguments", 1, test5);

    return 0;
}