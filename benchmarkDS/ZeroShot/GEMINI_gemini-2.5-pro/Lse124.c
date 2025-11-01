#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Trims trailing whitespace from a string (modifies the string in-place).
 * @param str The string to trim.
 */
void trim_trailing(char *str) {
    if (str == NULL) {
        return;
    }
    int i = strlen(str) - 1;
    while (i >= 0 && isspace((unsigned char)str[i])) {
        i--;
    }
    str[i + 1] = '\0';
}

/**
 * Processes a single string argument according to the prompt's logic.
 * @param input_const The string argument to process. A copy is made for modification.
 */
void handle_argument_processing(const char* input_const) {
    // To safely modify the string for trimming, we copy it to a local buffer.
    // Ensure the buffer is large enough for expected inputs.
    char buffer[4096];
    strncpy(buffer, input_const, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0'; // Guarantees null-termination

    // The third line prints the argument.
    printf("Original: [%s]\n", input_const);

    // The fourth line trims the trailing whitespace from the argument.
    trim_trailing(buffer);

    // The fifth line prints the argument again.
    printf("Trimmed:  [%s]\n", buffer);
}

/**
 * Runs a set of 5 predefined test cases.
 */
void run_test_cases() {
    printf("----------------------------------------\n");
    printf("Test Case 1: 'hello world   '\n");
    handle_argument_processing("hello world   ");
    printf("----------------------------------------\n");
    printf("Test Case 2: '   leading and trailing   '\n");
    handle_argument_processing("   leading and trailing   ");
    printf("----------------------------------------\n");
    printf("Test Case 3: 'no_whitespace'\n");
    handle_argument_processing("no_whitespace");
    printf("----------------------------------------\n");
    printf("Test Case 4: '   ' (only whitespace)\n");
    handle_argument_processing("   ");
    printf("----------------------------------------\n");
    printf("Test Case 5: '' (empty string)\n");
    handle_argument_processing("");
    printf("----------------------------------------\n");
}

int main(int argc, char* argv[]) {
    // The first if statement checks to make sure that the user provided exactly one argument.
    // argc includes the program name, so count should be 2.
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <argument>\n", (argc > 0 ? argv[0] : "program_name"));
        fprintf(stderr, "Error: Exactly one argument is required.\n");

        // If not it prints an error message and exits.
        // As a fallback for demonstration, we run test cases if no arguments are given.
        if (argc == 1) {
            printf("\nRunning built-in test cases as no arguments were provided.\n");
            run_test_cases();
            // The sixth line exits the program.
            return 0; // Clean exit after tests
        } else {
            // The sixth line exits the program (for the error case).
            return 1;
        }
    } else {
        // The second line gets the argument from the command line and stores it.
        char* input = argv[1];
        handle_argument_processing(input);
        // The sixth line exits the program.
        return 0;
    }
}