#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Define a maximum length for the input string to prevent buffer overflows.
#define MAX_STR_LEN 256

/**
 * Trims trailing whitespace from a C string in-place.
 *
 * @param str A pointer to the character array (string) to trim.
 *            The string must be modifiable and null-terminated.
 */
void trimTrailingWhitespace(char *str) {
    if (str == NULL) {
        return;
    }

    size_t len = strlen(str);
    if (len == 0) {
        return;
    }

    // The for loop trims the trailing whitespace in the string.
    // The second for loop trims the newlines.
    // (Combined into one loop for efficiency as isspace handles both.)
    // Note: The loop index must be a signed type to correctly handle the i < 0 check.
    long i = len - 1;
    while (i >= 0 && isspace((unsigned char)str[i])) {
        i--;
    }

    // The last line null terminates the string.
    str[i + 1] = '\0';
}

void run_test_case(const char* test_name, const char* input) {
    char buffer[MAX_STR_LEN];
    
    // Securely copy the input string to a modifiable buffer
    strncpy(buffer, input, MAX_STR_LEN - 1);
    buffer[MAX_STR_LEN - 1] = '\0'; // Ensure null termination

    printf("--- %s ---\n", test_name);
    printf("Original: \"%s\"\n", buffer);
    trimTrailingWhitespace(buffer);
    printf("Trimmed : \"%s\"\n\n", buffer);
}

int main(int argc, char *argv[]) {
    // The first if statement checks to make sure that the user provided exactly one argument.
    if (argc != 2) {
        fprintf(stderr, "Usage: %s \"<string to trim>\"\n", argv[0]);
        fprintf(stderr, "Running test cases instead.\n\n");

        // Running 5 test cases
        run_test_case("Test Case 1: Trailing spaces", "hello world  ");
        run_test_case("Test Case 2: Trailing mixed whitespace", "test\t\n \r");
        run_test_case("Test Case 3: No trailing whitespace", "already_clean");
        run_test_case("Test Case 4: All whitespace", "   \t\n");
        run_test_case("Test Case 5: Empty string", "");
        return 1;
    }
    
    // Check the argument to make sure that it is a string (argv elements are strings)
    const char *input = argv[1];
    
    // Check input length against our buffer size to prevent overflow
    size_t input_len = strlen(input);
    if (input_len >= MAX_STR_LEN) {
        fprintf(stderr, "Error: Input string is too long (max %d characters).\n", MAX_STR_LEN - 1);
        return 1;
    }
    
    // It is not safe to modify argv strings directly. Copy to a local, modifiable buffer.
    char buffer[MAX_STR_LEN];
    strncpy(buffer, input, MAX_STR_LEN);
    // strncpy may not null-terminate if src is >= dest size, so we ensure it.
    buffer[MAX_STR_LEN - 1] = '\0';

    printf("Original: \"%s\"\n", buffer);
    trimTrailingWhitespace(buffer);
    printf("Trimmed : \"%s\"\n", buffer);

    return 0;
}