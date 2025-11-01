#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * Converts the second command-line argument to an integer (INDEX) and prints
 * the argument at that INDEX.
 *
 * @param argc The count of command-line arguments.
 * @param argv The array of command-line arguments.
 */
void processAndPrintArgument(int argc, char* argv[]) {
    // 1. Check if the second argument (for the index) exists.
    if (argc < 2) {
        fprintf(stderr, "Error: Not enough arguments. Please provide an index.\n");
        return;
    }

    char* indexStr = argv[1];
    char* endptr;
    long index_long;

    // 2. Securely convert the string to a long integer using strtol.
    errno = 0; // Reset errno before the call
    index_long = strtol(indexStr, &endptr, 10);

    // Check for conversion errors
    if (endptr == indexStr) {
        fprintf(stderr, "Error: The provided index '%s' is not a valid number.\n", indexStr);
        return;
    }
    if (*endptr != '\0') {
        fprintf(stderr, "Error: The provided index '%s' contains non-numeric characters.\n", indexStr);
        return;
    }
    if ((errno == ERANGE && (index_long == LONG_MAX || index_long == LONG_MIN)) || 
        (index_long > INT_MAX || index_long < INT_MIN)) {
        fprintf(stderr, "Error: The provided index '%s' is out of the integer range.\n", indexStr);
        return;
    }

    int index = (int)index_long;

    // 3. Check if the parsed index is within the valid bounds of the arguments array.
    if (index < 0 || index >= argc) {
        fprintf(stderr, "Error: Index %d is out of bounds. Must be between 0 and %d.\n", index, argc - 1);
        return;
    }

    // 4. Print the argument at the specified index.
    printf("Argument at index %d: %s\n", index, argv[index]);
}

int main() {
    printf("--- Test Cases ---\n");

    // Test Case 1: Valid input
    printf("\nTest Case 1: ./lse7 2 Hello World\n");
    char* test1[] = {"./lse7", "2", "Hello", "World"};
    processAndPrintArgument(4, test1);

    // Test Case 2: Index out of bounds (too high)
    printf("\nTest Case 2: ./lse7 10 Hello\n");
    char* test2[] = {"./lse7", "10", "Hello"};
    processAndPrintArgument(3, test2);

    // Test Case 3: Invalid number format for index
    printf("\nTest Case 3: ./lse7 abc Hello\n");
    char* test3[] = {"./lse7", "abc", "Hello"};
    processAndPrintArgument(3, test3);

    // Test Case 4: Not enough arguments to get the index
    printf("\nTest Case 4: ./lse7\n");
    char* test4[] = {"./lse7"};
    processAndPrintArgument(1, test4);

    // Test Case 5: Integer overflow for the index
    printf("\nTest Case 5: ./lse7 2147483648\n"); // Assuming 32-bit int
    char* test5[] = {"./lse7", "2147483648"};
    processAndPrintArgument(2, test5);

    return 0;
}