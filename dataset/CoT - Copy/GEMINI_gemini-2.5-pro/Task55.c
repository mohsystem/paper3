#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h> // For POSIX regular expressions

/**
 * Validates an email address using POSIX regular expressions.
 *
 * @param email The C-string to validate.
 * @return true if the email is valid, false otherwise.
 */
bool isValidEmail(const char* email) {
    // A NULL or empty pointer is not a valid email
    if (email == NULL || *email == '\0') {
        return false;
    }

    regex_t regex;
    int reti;
    bool is_match = false;

    // Regex for standard email validation.
    const char* pattern = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";

    // Compile the regular expression. REG_EXTENDED allows for modern regex syntax.
    // REG_NOSUB is an optimization, as we don't need capture groups.
    reti = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return false; // Regex compilation failed
    }

    // Execute the regular expression.
    reti = regexec(&regex, email, 0, NULL, 0);
    if (!reti) {
        is_match = true; // Match found
    } else if (reti == REG_NOMATCH) {
        is_match = false; // No match
    } else {
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        is_match = false;
    }

    // Free the memory allocated by regcomp. This is crucial to prevent memory leaks.
    regfree(&regex);

    return is_match;
}

int main() {
    const char* testEmails[] = {
        "test@example.com",          // Valid
        "test.name+alias@example.co.uk", // Valid
        "plainaddress",              // Invalid
        "@missing-local-part.com",   // Invalid
        "test@domain@domain.com"     // Invalid
    };
    int num_tests = sizeof(testEmails) / sizeof(testEmails[0]);

    printf("C Email Validation:\n");
    for (int i = 0; i < num_tests; ++i) {
        printf("'%s' is valid: %s\n", testEmails[i], isValidEmail(testEmails[i]) ? "true" : "false");
    }

    return 0;
}