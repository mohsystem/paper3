#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * Securely retrieves an argument from the array based on an index provided in argv[1].
 *
 * @param argc The number of command-line arguments.
 * @param argv The array of command-line arguments.
 * @return The string at the specified index or a static error message.
 */
const char* getArgumentAtIndex(int argc, char* argv[]) {
    // Security Check 1: Ensure there are enough arguments to get an index.
    if (argc < 2) {
        return "Error: Not enough arguments. An index must be provided as the second argument.";
    }

    char* endptr;
    long index;
    
    // Security Check 2: Use strtol for safe string-to-long conversion.
    errno = 0; // Reset errno before the call
    index = strtol(argv[1], &endptr, 10);

    // Check for various conversion errors.
    if (endptr == argv[1] || *endptr != '\0') {
        return "Error: Index provided in the second argument must be a valid integer.";
    }
    if (errno == ERANGE) {
        return "Error: Index value is out of range for a long integer.";
    }
    // Check if the long value fits in a standard int for array indexing
    if (index > INT_MAX || index < INT_MIN) {
        return "Error: Index value is out of range for an integer index.";
    }

    // Security Check 3: Ensure the index is within the valid bounds of the array.
    if (index < 0) {
        return "Error: Index cannot be negative.";
    }
    if (index >= argc) {
        return "Error: Index is out of bounds.";
    }

    // If all checks pass, return the value at the specified index.
    return argv[index];
}

int main() {
    printf("--- C: Testing argument retrieval ---\n");

    // Test Case 1: Valid input
    char* test1_args[] = { (char*)"./Lse8", (char*)"3", (char*)"apple", (char*)"banana" };
    printf("Test Case 1 Input: {\"./Lse8\", \"3\", \"apple\", \"banana\"}\n");
    printf("Result: %s\n\n", getArgumentAtIndex(4, test1_args));

    // Test Case 2: Not enough arguments
    char* test2_args[] = { (char*)"./Lse8" };
    printf("Test Case 2 Input: {\"./Lse8\"}\n");
    printf("Result: %s\n\n", getArgumentAtIndex(1, test2_args));

    // Test Case 3: Invalid index format (not an integer)
    char* test3_args[] = { (char*)"./Lse8", (char*)"abc", (char*)"arg2", (char*)"arg3" };
    printf("Test Case 3 Input: {\"./Lse8\", \"abc\", \"arg2\", \"arg3\"}\n");
    printf("Result: %s\n\n", getArgumentAtIndex(4, test3_args));

    // Test Case 4: Index out of bounds (too high)
    char* test4_args[] = { (char*)"./Lse8", (char*)"10", (char*)"arg2" };
    printf("Test Case 4 Input: {\"./Lse8\", \"10\", \"arg2\"}\n");
    printf("Result: %s\n\n", getArgumentAtIndex(3, test4_args));

    // Test Case 5: Index out of bounds (negative)
    char* test5_args[] = { (char*)"./Lse8", (char*)"-1", (char*)"arg2", (char*)"arg3" };
    printf("Test Case 5 Input: {\"./Lse8\", \"-1\", \"arg2\", \"arg3\"}\n");
    printf("Result: %s\n", getArgumentAtIndex(4, test5_args));
    
    return 0;
}