#include <stdio.h>
#include <string.h>
#include <regex.h> // For POSIX regular expressions

/**
 * Validates an input string against a given regular expression using POSIX regex.
 *
 * @param input The string to validate.
 * @param regexPattern The regular expression pattern.
 * @return 1 for a match, 0 for no match, and -1 for a compilation or execution error.
 */
int validateInput(const char* input, const char* regexPattern) {
    if (input == NULL || regexPattern == NULL) {
        return -1;
    }

    regex_t regex;
    int reti;
    int result;

    // Compile the regular expression with extended syntax support
    reti = regcomp(&regex, regexPattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return -1; // Error
    }

    // Execute the regular expression
    reti = regexec(&regex, input, 0, NULL, 0);
    if (!reti) {
        result = 1; // Match
    } else if (reti == REG_NOMATCH) {
        result = 0; // No match
    } else {
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        result = -1; // Error
    }

    // Free the compiled regular expression object
    regfree(&regex);

    return result;
}

int main() {
    // A simple regex for email validation.
    // In C strings, backslashes must be escaped with another backslash.
    const char* emailRegex = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";

    const char* testCases[] = {
        "test@example.com",         // Valid
        "test.name@example.co.uk",  // Valid
        "test@.com",                // Invalid
        "test",                     // Invalid
        "test@example"              // Invalid
    };
    int numTestCases = sizeof(testCases) / sizeof(testCases[0]);

    printf("--- Testing Email Validation ---\n");
    for (int i = 0; i < numTestCases; i++) {
        int validationResult = validateInput(testCases[i], emailRegex);
        printf("Input: \"%s\", Is Valid? ", testCases[i]);
        if (validationResult == 1) {
            printf("true\n");
        } else if (validationResult == 0) {
            printf("false\n");
        } else {
            printf("Error\n");
        }
    }

    return 0;
}