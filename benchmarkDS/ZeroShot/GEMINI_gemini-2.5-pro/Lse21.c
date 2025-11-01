#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * @brief Parses a string, adds 1000 to it, and stores the result.
 * 
 * This function uses secure methods to parse the string, checking for format,
 * range, and potential overflow before addition.
 * @param inputStr The string representation of the number.
 * @param output A pointer to a long long where the result will be stored.
 * @return 0 on success, -1 on failure.
 */
int addThousand(const char* inputStr, long long* output) {
    char* endptr;
    long long number;

    if (inputStr == NULL) {
        fprintf(stderr, "Error: Input string is NULL.\n");
        return -1;
    }
    
    // Reset errno before the call to distinguish new errors from old ones.
    errno = 0;
    number = strtoll(inputStr, &endptr, 10);

    // Secure checks for conversion errors.
    if (endptr == inputStr) {
        fprintf(stderr, "Error: No digits were found in input.\n");
        return -1;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: Input contains non-numeric characters.\n");
        return -1;
    }
    if (errno == ERANGE) {
        fprintf(stderr, "Error: Input number is out of range for a 64-bit integer.\n");
        return -1;
    }

    // Secure check for potential overflow before performing addition.
    if (number > LLONG_MAX - 1000) {
        fprintf(stderr, "Error: Adding 1000 would cause an overflow.\n");
        return -1;
    }

    *output = number + 1000;
    return 0;
}

/**
 * @brief Main entry point. Reads a single command-line argument and processes it.
 * Includes 5 test cases in the comments demonstrating how to run the program.
 */
int main(int argc, char* argv[]) {
    /*
     * How to compile and run:
     * 1. Compile: gcc -std=c99 -o Lse21 <source_file>.c
     * 2. Run:     ./Lse21 <value>
     *
     * Test Cases:
     * 1. Normal positive value: ./Lse21 50
     *    Expected output: 1050
     * 2. Normal negative value: ./Lse21 -2000
     *    Expected output: -1000
     * 3. Zero value:            ./Lse21 0
     *    Expected output: 1000
     * 4. Invalid input (text):  ./Lse21 "hello"
     *    Expected output: Error: No digits were found in input.
     * 5. Overflow value:        ./Lse21 9223372036854775807
     *    Expected output: Error: Adding 1000 would cause an overflow.
    */
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <integer_value>\n", argv[0]);
        return 1; // Return non-zero for error
    }

    long long result;
    if (addThousand(argv[1], &result) == 0) {
        printf("%lld\n", result);
    } else {
        // Error message is already printed by the function.
        return 1;
    }

    return 0;
}