#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/**
 * Takes command line arguments, parses the first argument as an index,
 * and prints the argument at that raw index.
 *
 * @param argc The argument count.
 * @param argv The argument vector.
 */
void printArgumentByIndex(int argc, char* argv[]) {
    // argv[0] is the program name. Need at least argv[1] (the index).
    if (argc < 2) {
        fprintf(stderr, "Error: No index provided. Usage: ./program <index> [args...]\n");
        return;
    }

    char* endptr;
    errno = 0; // To distinguish success/failure after call
    long index = strtol(argv[1], &endptr, 10);

    // Check for conversion errors
    if (endptr == argv[1] || *endptr != '\0') {
        fprintf(stderr, "Error: Invalid index '%s'. Index must be an integer.\n", argv[1]);
        return;
    }
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Index '%s' is out of range for a long integer.\n", argv[1]);
        return;
    }

    // The user-provided index N corresponds to argv[N].
    // We must check if this index is valid for the argv array.
    if (index >= 0 && index < argc) {
        printf("%s\n", argv[index]);
    } else {
        fprintf(stderr, "Error: Index %ld is out of bounds for arguments array of size %d.\n", index, argc);
    }
}

int main(int argc, char* argv[]) {
    // This program processes actual command-line arguments.
    // The following comments show 5 test cases and how to run them from the terminal.

    /*
     * To compile and run:
     * gcc -o program your_source.c
     *
     * --- Test Cases ---
     *
     * 1. Normal Case: Get the argument at index 3.
     *    Command: ./program 3 apple banana cherry
     *    Expected Output: cherry
     *    (argv is ['./program', '3', 'apple', 'banana', 'cherry'], argv[3] is 'cherry')
     *
     * 2. Edge Case (Program Name): Get the argument at index 0.
     *    Command: ./program 0 apple banana
     *    Expected Output: ./program
     *
     * 3. Error Case (Index Out of Bounds): Index is too high.
     *    Command: ./program 5 apple banana
     *    Expected Output: Error: Index 5 is out of bounds for arguments array of size 4.
     *
     * 4. Error Case (Invalid Index): Index is not a number.
     *    Command: ./program foo apple banana
     *    Expected Output: Error: Invalid index 'foo'. Index must be an integer.
     *
     * 5. Error Case (Index points to itself): Get the argument at index 1.
     *    Command: ./program 1 apple banana
     *    Expected Output: 1
     */

    printArgumentByIndex(argc, argv);
    return 0;
}