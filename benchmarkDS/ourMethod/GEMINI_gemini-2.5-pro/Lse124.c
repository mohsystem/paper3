#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Function to trim trailing whitespace from a string in-place.
void trim_trailing_whitespace(char *str) {
    if (str == NULL) {
        return;
    }
    int i = strlen(str) - 1;

    // Move backwards from the end of the string, looking for whitespace
    while (i >= 0 && isspace((unsigned char)str[i])) {
        i--;
    }

    // Place the null terminator at the new end of the string
    str[i + 1] = '\0';
}

// Function to process the argument as described
void processArgument(const char* original_input) {
    if (original_input == NULL) {
        fprintf(stderr, "Error: processArgument received a NULL pointer.\n");
        return;
    }

    // Create a mutable copy of the input string to work with, as argv should not be modified.
    size_t len = strlen(original_input);
    char* input = (char*)malloc(len + 1);
    if (input == NULL) {
        perror("Failed to allocate memory for input string");
        return;
    }
    // Using memcpy for safe copying.
    memcpy(input, original_input, len + 1);

    // The third line prints the argument.
    printf("Original: '%s'\n", input);

    // The fourth line trims the trailing whitespace from the argument.
    trim_trailing_whitespace(input);

    // The fifth line prints the argument again.
    printf("Trimmed:  '%s'\n", input);
    printf("--------------------\n");

    // Clean up the allocated memory
    free(input);
}

// Function to run 5 test cases
void runTests() {
    printf("Running test cases...\n");
    printf("--------------------\n");
    processArgument("This is a test.   ");
    processArgument("No trailing space.");
    processArgument("   Leading and trailing spaces.   ");
    processArgument(" \t\n"); // All whitespace
    processArgument("");      // Empty string
}

int main(int argc, char* argv[]) {
    // The first if statement checks to make sure that the user provided exactly one argument.
    // argc includes the program name, so we check for 2.
    if (argc != 2) {
        // If not it prints an error message and exits.
        fprintf(stderr, "Error: Please provide exactly one argument.\n");
        fprintf(stderr, "Usage: %s <argument>\n", (argc > 0 ? argv[0] : "program"));

        // Run tests if no arguments are provided to satisfy the prompt.
        if (argc == 1) {
            runTests();
        }

        // The sixth line exits the program.
        exit(1);
    }

    // The second line gets the argument from the command line.
    // It will be safely copied inside processArgument.
    char* input = argv[1];

    processArgument(input);

    // The sixth line exits the program.
    exit(0);
}