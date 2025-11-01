#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Processes a single command-line argument according to the specified logic.
 * @param argc The argument count.
 * @param argv The argument values.
 * @return 0 on success, 1 on error.
 */
int main(int argc, char* argv[]) {
    // 1) Check for exactly one argument. If not, print an error and exit.
    // argc includes the program name, so it should be 2 for one user argument.
    if (argc != 2) {
        fprintf(stderr, "Error: Please provide exactly one argument.\n");
        return 1; // Exit with an error code
    }

    // 2) Get the argument and store it.
    char* input = argv[1];

    // 3) Print the original argument.
    printf("Original: '%s'\n", input);

    // 4) Trim the trailing whitespace.
    // Create a mutable copy to modify.
    char* trimmedInput = strdup(input);
    if (trimmedInput == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        return 1;
    }
    
    int len = strlen(trimmedInput);
    char* end = trimmedInput + len - 1;
    
    // Move 'end' pointer back over trailing whitespace characters.
    while (end >= trimmedInput && isspace((unsigned char)*end)) {
        end--;
    }
    
    // Null-terminate the string at the new end.
    *(end + 1) = '\0';

    // 5) Print the trimmed argument.
    printf("Trimmed:  '%s'\n", trimmedInput);
    
    // Free the allocated memory for the copy.
    free(trimmedInput);

    // 6) Exit the program.
    return 0; // Exit successfully

    /*
     * Main method with 5 test cases (to be run from the command line).
     *
     * How to compile and run:
     * gcc your_source_file.c -o Lse124
     *
     * Test Case 1: Argument with trailing spaces
     * ./Lse124 "hello world   "
     * Expected Output:
     * Original: 'hello world   '
     * Trimmed:  'hello world'
     *
     * Test Case 2: Argument with leading and trailing spaces
     * ./Lse124 "  leading and trailing  "
     * Expected Output:
     * Original: '  leading and trailing  '
     * Trimmed:  '  leading and trailing'
     *
     * Test Case 3: Argument with no trailing whitespace
     * ./Lse124 "no-whitespace"
     * Expected Output:
     * Original: 'no-whitespace'
     * Trimmed:  'no-whitespace'
     *
     * Test Case 4: No arguments (error case)
     * ./Lse124
     * Expected Output:
     * Error: Please provide exactly one argument.
     *
     * Test Case 5: Multiple arguments (error case)
     * ./Lse124 "one" "two"
     * Expected Output:
     * Error: Please provide exactly one argument.
     */
}