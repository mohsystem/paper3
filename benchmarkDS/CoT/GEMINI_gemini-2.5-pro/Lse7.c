#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

/**
 * Safely converts a string to an integer.
 * @param s The string to convert.
 * @param out_val A pointer to store the converted integer.
 * @return 1 if conversion is successful, 0 otherwise.
 */
int convertStringToInt(const char* s, int* out_val) {
    char* endptr;
    long val;

    if (s == NULL || *s == '\0') {
        return 0; // Invalid input string
    }

    errno = 0; // To distinguish success/failure after call
    val = strtol(s, &endptr, 10);

    // Check for various possible errors
    if (endptr == s) {
        return 0; // No digits were found
    }
    if (*endptr != '\0') {
        return 0; // Trailing characters that are not part of the number
    }
    if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN)) || (val > INT_MAX || val < INT_MIN)) {
        return 0; // Out of range for an int
    }

    *out_val = (int)val;
    return 1;
}

/**
 * Prints the command line argument at the index specified by the second argument.
 * @param argc The number of command line arguments.
 * @param argv The array of command line argument strings.
 */
void printArgumentAtIndex(int argc, char* argv[]) {
    // 1. Check if there are enough arguments (program name + index)
    if (argc < 2) {
        fprintf(stderr, "Error: Not enough arguments. Usage: <program> <index> [arg1] [arg2] ...\n");
        return;
    }

    int index;
    // 2. Convert the second argument (at index 1) to an integer
    if (!convertStringToInt(argv[1], &index)) {
        fprintf(stderr, "Error: The second argument '%s' must be a valid integer.\n", argv[1]);
        return;
    }

    // 3. Check if the calculated index is within the bounds of the argv array
    if (index >= 0 && index < argc) {
        // 4. Print the argument at the specified index
        printf("%s\n", argv[index]);
    } else {
        fprintf(stderr, "Error: Index %d is out of bounds. Valid range is 0 to %d.\n", index, argc - 1);
    }
}

int main(int argc, char* argv[]) {
    printf("--- Running with actual command line arguments ---\n");
    // To test this part, compile and run from command line:
    // ./a.out 3 apple banana orange grape
    // Expected output: grape
    if (argc > 1) {
        printArgumentAtIndex(argc, argv);
    } else {
        printf("No command line arguments provided for live test.\n");
    }
    
    printf("\n--- Running with 5 test cases ---\n");

    // Test Case 1: Valid input
    printf("Test 1: ");
    char* test1[] = {"program", "3", "zero", "one", "two", "three"};
    printArgumentAtIndex(6, test1); // Expected: one

    // Test Case 2: Index out of bounds (too high)
    printf("Test 2: ");
    char* test2[] = {"program", "5", "a", "b", "c"};
    printArgumentAtIndex(5, test2); // Expected: Error: Index 5 is out of bounds.

    // Test Case 3: Index is not a number
    printf("Test 3: ");
    char* test3[] = {"program", "two", "a", "b", "c"};
    printArgumentAtIndex(5, test3); // Expected: Error: The second argument 'two' must be a valid integer.

    // Test Case 4: Not enough arguments
    printf("Test 4: ");
    char* test4[] = {"program"};
    printArgumentAtIndex(1, test4); // Expected: Error: Not enough arguments.

    // Test Case 5: Index is negative
    printf("Test 5: ");
    char* test5[] = {"program", "-1", "a", "b"};
    printArgumentAtIndex(4, test5); // Expected: Error: Index -1 is out of bounds.

    return 0;
}