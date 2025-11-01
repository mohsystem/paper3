#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

const int OFFSET = 1000;

/**
 * @brief Adds a constant offset to an integer value, checking for overflow.
 * 
 * @param value The integer to add the offset to.
 * @param result A pointer to an integer where the result will be stored.
 * @return 0 on success, -1 on failure (e.g., overflow).
 */
int add_offset(int value, int* result) {
    if (result == NULL) {
        return -1;
    }
    // Check for potential positive overflow before addition.
    if (value > 0 && value > INT_MAX - OFFSET) {
        fprintf(stderr, "Error: Integer overflow detected on addition.\n");
        return -1;
    }
    *result = value + OFFSET;
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <integer_value>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char* endptr;
    long value;

    errno = 0; // To distinguish success/failure after call to strtol
    value = strtol(argv[1], &endptr, 10);

    // Check for various conversion errors from strtol
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Input value is out of range for a long.\n");
        return EXIT_FAILURE;
    }
    if (endptr == argv[1]) {
        fprintf(stderr, "Error: No digits were found in input.\n");
        return EXIT_FAILURE;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Trailing non-numeric characters in input.\n");
        return EXIT_FAILURE;
    }

    // After successful parsing to long, check if it fits in an int
    if (value > INT_MAX || value < INT_MIN) {
        fprintf(stderr, "Error: Input value is out of range for an integer.\n");
        return EXIT_FAILURE;
    }

    int int_value = (int)value;
    int result;

    if (add_offset(int_value, &result) != 0) {
        // The error message is printed inside the function
        return EXIT_FAILURE;
    }
    
    printf("Result: %d\n", result);

    /*
    To run the test cases, compile the code first:
    gcc -o lse20 lse20.c -Wall -Werror

    Then run from the command line:

    Test Case 1: Positive number
    Command: ./lse20 50
    Expected Output: Result: 1050

    Test Case 2: Negative number
    Command: ./lse20 -50
    Expected Output: Result: 950

    Test Case 3: Zero
    Command: ./lse20 0
    Expected Output: Result: 1000

    Test Case 4: Invalid input
    Command: ./lse20 abc
    Expected Output: Error: No digits were found in input.

    Test Case 5: Overflow
    Command: ./lse20 2147483647
    Expected Output: Error: Integer overflow detected on addition.
    */

    return EXIT_SUCCESS;
}