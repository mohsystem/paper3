#include <stdio.h>
#include <stdbool.h>
#include <regex.h>
#include <string.h>

/**
 * Validates an email address using POSIX regular expressions.
 *
 * @param email The email string to validate.
 * @return true if the email is valid, false otherwise.
 */
bool isValidEmail(const char* email) {
    if (email == NULL) {
        return false;
    }
    
    regex_t regex;
    int reti;
    bool isValid = false;

    // A common regex for email validation.
    const char* pattern = "^[a-zA-Z0-9_+&*-]\\+\\(\\.\\([a-zA-Z0-9_+&*-]\\+\\)\\)*@\\([a-zA-Z0-9-]*\\.\\)\\+[a-zA-Z]\\{2,7\\}$";

    // Compile the regular expression using ERE (Extended Regular Expression) syntax
    reti = regcomp(&regex, pattern, REG_EXTENDED | REG_NOSUB);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return false;
    }

    // Execute the regular expression
    reti = regexec(&regex, email, 0, NULL, 0);
    if (!reti) {
        isValid = true; // Match found
    } else if (reti == REG_NOMATCH) {
        isValid = false; // No match
    } else {
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
        isValid = false;
    }

    // Free the compiled regular expression
    regfree(&regex);
    
    return isValid;
}

int main() {
    const char* testEmails[] = {
        "test.user@example.com",      // Valid
        "user@sub.domain.co.uk",  // Valid
        "invalid-email",              // Invalid (no @)
        "user@.com",                  // Invalid (domain starts with a dot)
        "user@domain..com"            // Invalid (consecutive dots in domain)
    };
    
    int numTests = sizeof(testEmails) / sizeof(testEmails[0]);

    printf("--- C Email Validation ---\n");
    for (int i = 0; i < numTests; ++i) {
        printf("Email: %s -> Is Valid: %s\n", testEmails[i], isValidEmail(testEmails[i]) ? "true" : "false");
    }

    return 0;
}