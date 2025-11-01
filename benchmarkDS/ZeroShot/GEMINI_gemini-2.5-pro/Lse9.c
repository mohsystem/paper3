#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h> // For errno and ERANGE
#include <limits.h> // For LONG_MIN, LONG_MAX

/**
 * Retrieves a command-line argument by a 1-based index.
 *
 * @param argc The argument count.
 * @param argv The argument vector. argv[1] is the index, argv[2...] are data.
 * @param result A pointer to a char pointer, which will be set to the result on success.
 * @param error_msg A pointer to a char pointer, which will be set to an error message on failure.
 * @return 0 on success, non-zero on failure.
 */
int getArgumentByIndex(int argc, char* argv[], char** result, const char** error_msg) {
    // 1. Security Check: Ensure enough arguments
    // We need at least 3: program_name, index, data_arg1
    if (argc < 3) {
        *error_msg = "Usage: ./Lse9 <index> <arg1> [arg2] ...";
        return 1;
    }

    char* endptr;
    long index;
    
    // 2. Security Check: Use strtol for safe string-to-long conversion
    errno = 0; // Reset errno before the call
    index = strtol(argv[1], &endptr, 10);

    // Check for various conversion errors
    if (errno == ERANGE) {
        *error_msg = "Error: Index is out of range for a long integer.";
        return 2;
    }
    // If no digits were found or if there are non-digit characters after the number
    if (endptr == argv[1] || *endptr != '\0') {
        *error_msg = "Error: Index must be a valid integer.";
        return 3;
    }

    // 3. Security Check: Ensure index is positive
    if (index < 1) {
        *error_msg = "Error: Index must be a positive integer (1-based).";
        return 4;
    }
    
    // Check if the parsed long value can fit in an int, as we use it for indexing
    if (index > INT_MAX) {
        *error_msg = "Error: Index is too large.";
        return 5;
    }

    int int_index = (int)index;

    // Number of data arguments available
    int dataArgCount = argc - 2;

    // 4. Security Check: Ensure index is within bounds
    if (int_index > dataArgCount) {
        // Note: For C, creating a dynamic error string is complex (memory management).
        // A static but less specific message is often safer and simpler.
        *error_msg = "Error: Index is out of bounds.";
        return 6;
    }

    // The user's 1-based index `index` corresponds to argv[index + 1]
    *result = argv[int_index + 1];
    return 0; // Success
}

int main(int argc, char* argv[]) {
    // Note: These test cases are for demonstration.
    // You must run this program from the command line to provide arguments.

    /*
    To compile and run from the command line:
    gcc -Wall -Wextra -o Lse9 Lse9.c

    Test Cases:
    1. Valid case: Get the 2nd data argument
       Command: ./Lse9 2 apple banana orange
       Expected Output: banana

    2. Valid case: Get the 1st data argument
       Command: ./Lse9 1 apple banana orange
       Expected Output: apple

    3. Error case: Index out of bounds
       Command: ./Lse9 4 apple banana orange
       Expected Output: Error: Index is out of bounds.

    4. Error case: Non-integer index
       Command: ./Lse9 two apple banana
       Expected Output: Error: Index must be a valid integer.

    5. Error case: Not enough arguments
       Command: ./Lse9 1
       Expected Output: Usage: ./Lse9 <index> <arg1> [arg2] ...
    */

    char* argument = NULL;
    const char* error_message = NULL;

    int status = getArgumentByIndex(argc, argv, &argument, &error_message);

    if (status == 0) {
        printf("%s\n", argument);
        return 0; // Success
    } else {
        fprintf(stderr, "%s\n", error_message);
        return status; // Failure
    }
}