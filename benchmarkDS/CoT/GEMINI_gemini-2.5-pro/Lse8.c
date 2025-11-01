#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // For errno
#include <limits.h> // For LONG_MIN, LONG_MAX

/**
 * Retrieves the command-line argument at the index specified by the first argument.
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of C-style string arguments.
 * @return The argument at the specified index, or NULL if an error occurs.
 */
const char* getValueAtIndex(int argc, char* argv[]) {
    // Security Check 1: Ensure there are enough arguments.
    if (argc < 2) {
        fprintf(stderr, "Error: Not enough arguments. Usage: ./program <index> <arg1> ...\n");
        return NULL;
    }

    char* endptr;
    const char* index_str = argv[1];
    long index;

    // Security Check 2: Safely parse the index using strtol, which is safer than atoi.
    errno = 0; // Reset errno before the call
    index = strtol(index_str, &endptr, 10);

    // Check for conversion errors.
    if (endptr == index_str) {
        fprintf(stderr, "Error: The first argument ('%s') is not a valid number.\n", index_str);
        return NULL;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: The first argument ('%s') contains non-numeric characters.\n", index_str);
        return NULL;
    }
    if ((errno == ERANGE && (index == LONG_MAX || index == LONG_MIN)) || (index > INT_MAX || index < INT_MIN)) {
        fprintf(stderr, "Error: The first argument ('%s') is out of the integer range.\n", index_str);
        return NULL;
    }
    
    int final_index = (int)index;

    // Security Check 3: Perform bounds checking.
    if (final_index < 0 || final_index >= argc) {
        fprintf(stderr, "Error: Index %d is out of bounds. Valid range is [0, %d].\n", final_index, argc - 1);
        return NULL;
    }

    // If all checks pass, return the value.
    return argv[final_index];
}

int main(int argc, char* argv[]) {
    const char* result = getValueAtIndex(argc, argv);

    if (result != NULL) {
        printf("Value at index %s: %s\n", argv[1], result);
        return 0; // Success
    } else {
        return 1; // Failure
    }

    // --- Test Cases ---
    // This program must be compiled and run from the command line.
    //
    // Compile first: gcc -Wall your_file.c -o program
    //
    // Test Case 1: Successful retrieval
    // Command: ./program 3 apple banana orange grape
    // Expected Output: Value at index 3: orange
    //
    // Test Case 2: Index points to the program name itself (index 0)
    // Command: ./program 0 test one two
    // Expected Output: Value at index 0: ./program
    //
    // Test Case 3: Error - Index out of bounds
    // Command: ./program 5 apple banana
    // Expected Output: Error: Index 5 is out of bounds. Valid range is [0, 3].
    //
    // Test Case 4: Error - Invalid index format
    // Command: ./program two apple banana
    // Expected Output: Error: The first argument ('two') is not a valid number.
    //
    // Test Case 5: Error - Not enough arguments
    // Command: ./program
    // Expected Output: Error: Not enough arguments. Usage: ./program <index> <arg1> ...
}