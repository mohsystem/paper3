#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/**
 * @brief Trims trailing whitespace from a string in-place.
 * 
 * @param s A pointer to the character array (string) to be modified.
 *          This string MUST be mutable (e.g., allocated on the stack or heap).
 *          Passing a string literal will result in undefined behavior.
 */
void trimTrailingWhitespace(char *s) {
    if (s == NULL) {
        return;
    }

    int len = strlen(s);
    if (len == 0) {
        return;
    }

    char *end = s + len - 1;

    // Move `end` pointer backwards as long as it points to a whitespace character
    // and we haven't gone past the start of the string.
    while (end >= s && isspace((unsigned char)*end)) {
        end--;
    }

    // Place a new null-terminator after the last non-whitespace character.
    *(end + 1) = '\0';
}

/**
 * @brief Runs a set of hardcoded test cases to demonstrate the trimming function.
 */
void run_test_cases() {
    printf("No command-line arguments provided. Running test cases:\n");
    
    // Test strings must be declared as arrays to be mutable.
    char test1[] = "Hello World   ";
    char test2[] = "  leading and trailing  \t\n";
    char test3[] = "NoWhitespace";
    char test4[] = "   ";
    char test5[] = "";
    
    // Array of pointers to the test strings for easy iteration.
    char* tests[] = {test1, test2, test3, test4, test5};
    const char* originals[] = {
        "Hello World   ",
        "  leading and trailing  \t\n",
        "NoWhitespace",
        "   ",
        ""
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);
    for (int i = 0; i < num_tests; i++) {
        printf("Original: \"%s\" -> ", originals[i]);
        trimTrailingWhitespace(tests[i]);
        printf("Trimmed: \"%s\"\n", tests[i]);
    }
}

int main(int argc, char *argv[]) {
    // If command-line arguments are provided, process them.
    if (argc > 1) {
        printf("Processing command-line arguments:\n");
        // argv[0] is the program name, so we start from index 1.
        for (int i = 1; i < argc; i++) {
            // Security: Do not modify argv strings directly. Make a mutable copy.
            // Allocate memory for the copy.
            char *arg_copy = malloc(strlen(argv[i]) + 1);
            if (arg_copy == NULL) {
                perror("Failed to allocate memory");
                return 1; // Exit with an error code
            }
            strcpy(arg_copy, argv[i]);

            printf("Original: \"%s\" -> ", argv[i]);
            trimTrailingWhitespace(arg_copy);
            printf("Trimmed: \"%s\"\n", arg_copy);

            // Security: Free the allocated memory to prevent leaks.
            free(arg_copy);
        }
    } else {
        // Otherwise, run the built-in test cases.
        run_test_cases();
    }

    return 0;
}