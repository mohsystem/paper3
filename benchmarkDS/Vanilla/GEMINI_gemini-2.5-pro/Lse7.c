#include <stdio.h>
#include <stdlib.h> // For strtol
#include <errno.h>  // For errno

/**
 * Converts the string at index 1 of the argv array to an integer 'INDEX',
 * then prints the argument at position 'INDEX'.
 *
 * @param argc The argument count.
 * @param argv An array of C-style strings (char*).
 */
void printArgFromIndex(int argc, char* argv[]) {
    // Check if there are enough arguments to proceed. We need at least 2.
    if (argc <= 1) {
        printf("Error: Not enough arguments. Please provide an index.\n");
        return;
    }

    char* endptr;
    errno = 0; // Reset errno before the call
    long index_long = strtol(argv[1], &endptr, 10);

    // Check for conversion errors.
    if (endptr == argv[1]) {
        printf("Error: The second argument is not a valid number.\n");
        return;
    }
    if (*endptr != '\0') {
        printf("Error: The second argument contains non-numeric characters.\n");
        return;
    }
    if (errno == ERANGE) {
        printf("Error: The index value is out of range for a long integer.\n");
        return;
    }

    int index = (int)index_long;
    
    // Check if the parsed index is within the valid bounds of the array.
    if (index < 0 || index >= argc) {
        printf("Error: Index %d is out of bounds for the given arguments.\n", index);
        return;
    }

    // Print the argument at the specified index.
    printf("%s\n", argv[index]);
}

int main() {
    // --- 5 Test Cases ---

    // Test Case 1: Valid case, should print "banana"
    printf("--- Test Case 1 ---\n");
    printf("Input: {\"./Lse7\", \"3\", \"apple\", \"banana\", \"cherry\"}\n");
    printf("Output: ");
    char* test1_argv[] = {"./Lse7", "3", "apple", "banana", "cherry"};
    int test1_argc = sizeof(test1_argv) / sizeof(char*);
    printArgFromIndex(test1_argc, test1_argv);
    printf("\n");

    // Test Case 2: Valid case, index 0, should print the program name
    printf("--- Test Case 2 ---\n");
    printf("Input: {\"./Lse7\", \"0\"}\n");
    printf("Output: ");
    char* test2_argv[] = {"./Lse7", "0"};
    int test2_argc = sizeof(test2_argv) / sizeof(char*);
    printArgFromIndex(test2_argc, test2_argv);
    printf("\n");

    // Test Case 3: Error case, index out of bounds
    printf("--- Test Case 3 ---\n");
    printf("Input: {\"./Lse7\", \"5\", \"a\", \"b\"}\n");
    printf("Output: ");
    char* test3_argv[] = {"./Lse7", "5", "a", "b"};
    int test3_argc = sizeof(test3_argv) / sizeof(char*);
    printArgFromIndex(test3_argc, test3_argv);
    printf("\n");

    // Test Case 4: Error case, non-integer index
    printf("--- Test Case 4 ---\n");
    printf("Input: {\"./Lse7\", \"two\", \"a\", \"b\"}\n");
    printf("Output: ");
    char* test4_argv[] = {"./Lse7", "two", "a", "b"};
    int test4_argc = sizeof(test4_argv) / sizeof(char*);
    printArgFromIndex(test4_argc, test4_argv);
    printf("\n");

    // Test Case 5: Error case, not enough arguments
    printf("--- Test Case 5 ---\n");
    printf("Input: {\"./Lse7\"}\n");
    printf("Output: ");
    char* test5_argv[] = {"./Lse7"};
    int test5_argc = sizeof(test5_argv) / sizeof(char*);
    printArgFromIndex(test5_argc, test5_argv);
    printf("\n");
    
    return 0;
}