#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Processes a single command-line argument based on the prompt's logic.
 *
 * @param argc The argument count.
 * @param argv The argument vector.
 */
void processArgument(int argc, char* argv[]) {
    // 1. Check if exactly one argument is provided. (argc should be 2: program name + arg)
    if (argc != 2) {
        fprintf(stderr, "Error: Please provide exactly one argument.\n");
        // 6. Exit the logical flow.
        return;
    }

    // 2. Get the argument.
    char* input = argv[1];

    // 3. Print the original argument.
    printf("Original: '%s'\n", input);

    // Create a modifiable copy of the input string
    char* trimmed_input = malloc(strlen(input) + 1);
    if (trimmed_input == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return;
    }
    strcpy(trimmed_input, input);

    // 4. Trim the trailing whitespace from the argument.
    int i = strlen(trimmed_input) - 1;
    while (i >= 0 && isspace((unsigned char)trimmed_input[i])) {
        trimmed_input[i] = '\0';
        i--;
    }

    // 5. Print the argument again.
    printf("Trimmed: '%s'\n", trimmed_input);

    // Clean up the allocated memory
    free(trimmed_input);

    // 6. The function ends, effectively exiting this logical block.
}

int main() {
    printf("--- Test Case 1: No arguments ---\n");
    char* argv1[] = { (char*)"program", NULL };
    processArgument(1, argv1);

    printf("\n--- Test Case 2: Two arguments ---\n");
    char* argv2[] = { (char*)"program", (char*)"first", (char*)"second", NULL };
    processArgument(3, argv2);

    printf("\n--- Test Case 3: One argument, no trailing space ---\n");
    char* argv3[] = { (char*)"program", (char*)"hello", NULL };
    processArgument(2, argv3);

    printf("\n--- Test Case 4: One argument with trailing spaces ---\n");
    char* argv4[] = { (char*)"program", (char*)"world  ", NULL };
    processArgument(2, argv4);

    printf("\n--- Test Case 5: One argument with leading and trailing spaces ---\n");
    char* argv5[] = { (char*)"program", (char*)"  spaced out   ", NULL };
    processArgument(2, argv5);

    return 0;
}