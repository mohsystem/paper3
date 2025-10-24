#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

/**
 * Validates a string against a given POSIX regular expression pattern.
 *
 * @param input The null-terminated string to validate.
 * @param regexPattern The null-terminated regular expression pattern.
 * @return 1 if the input matches the pattern, 0 otherwise.
 */
int validateInput(const char *input, const char *regexPattern) {
    if (input == NULL || regexPattern == NULL) {
        return 0;
    }

    regex_t regex;
    int reti;
    int result = 0;

    // Compile the regular expression
    // REG_EXTENDED uses modern regex syntax.
    // REG_NOSUB disables capturing groups for a slight performance gain.
    reti = regcomp(&regex, regexPattern, REG_EXTENDED | REG_NOSUB);
    if (reti) {
        char error_message[100];
        regerror(reti, &regex, error_message, sizeof(error_message));
        fprintf(stderr, "Regex compilation failed: %s\n", error_message);
        return 0; // Compilation failed
    }

    // Execute the regular expression
    reti = regexec(&regex, input, 0, NULL, 0);
    if (!reti) {
        result = 1; // Match found
    } else if (reti == REG_NOMATCH) {
        result = 0; // No match
    } else {
        char error_message[100];
        regerror(reti, &regex, error_message, sizeof(error_message));
        fprintf(stderr, "Regex match failed: %s\n", error_message);
        result = 0; // Match failed due to an error
    }

    // Free the compiled regular expression
    regfree(&regex);

    return result;
}

int main() {
    // Test Cases
    const char *inputs[] = {
        "test@example.com",
        "123-456-7890",
        "Alpha123",
        "Alpha123!",
        "not-an-email"
    };
    // Note: C POSIX regex does not use `\d`. We use `[0-9]` instead.
    const char *patterns[] = {
        "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$", // Email
        "^[0-9]{3}-[0-9]{3}-[0-9]{4}$",                      // US Phone Number
        "^[a-zA-Z0-9]+$",                                   // Alphanumeric
        "^[a-zA-Z0-9]+$",                                   // Alphanumeric (fail case)
        "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$"  // Email (fail case)
    };
    int expected[] = {1, 1, 1, 0, 0};
    int num_tests = sizeof(inputs) / sizeof(inputs[0]);

    printf("--- C Test Cases ---\n");
    for (int i = 0; i < num_tests; i++) {
        int result = validateInput(inputs[i], patterns[i]);
        printf("Test Case %d:\n", i + 1);
        printf("Input: \"%s\"\n", inputs[i]);
        printf("Pattern: \"%s\"\n", patterns[i]);
        printf("Result: %d | Expected: %d\n", result, expected[i]);
        printf("Status: %s\n", (result == expected[i] ? "Passed" : "Failed"));
        printf("\n");
    }

    return 0;
}