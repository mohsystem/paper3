#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <regex.h>

/**
 * Validates a string against a given regular expression pattern.
 *
 * @param input The string to validate.
 * @param regexPattern The regular expression pattern.
 * @return true if the input matches the pattern, false otherwise.
 */
bool validateInput(const char* input, const char* regexPattern) {
    if (input == NULL || regexPattern == NULL) {
        return false;
    }

    regex_t regex;
    int reti;
    bool is_match = false;

    // Compile the regular expression
    reti = regcomp(&regex, regexPattern, REG_EXTENDED);
    if (reti) {
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex compilation failed: %s\n", msgbuf);
        return false;
    }

    // Execute the regular expression
    reti = regexec(&regex, input, 0, NULL, 0);
    if (reti == 0) {
        is_match = true;
    } else if (reti != REG_NOMATCH) {
        char msgbuf[100];
        regerror(reti, &regex, msgbuf, sizeof(msgbuf));
        fprintf(stderr, "Regex match failed: %s\n", msgbuf);
    }
    
    // Free the compiled regular expression
    regfree(&regex);

    return is_match;
}

int main() {
    // Test cases
    const char* emailRegex = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,6}$";
    const char* phoneRegex = "^\\(?([0-9]{3})\\)?[-. ]?([0-9]{3})[-. ]?([0-9]{4})$";
    const char* usernameRegex = "^[a-zA-Z0-9]{5,15}$";

    // Test Case 1: Valid Email
    const char* validEmail = "test.user@example.com";
    bool isEmailValid = validateInput(validEmail, emailRegex);
    printf("1. Is '%s' a valid email? %s\n", validEmail, isEmailValid ? "true" : "false");

    // Test Case 2: Invalid Email
    const char* invalidEmail = "invalid-email@.com";
    bool isEmailInvalid = validateInput(invalidEmail, emailRegex);
    printf("2. Is '%s' a valid email? %s\n", invalidEmail, isEmailInvalid ? "true" : "false");

    // Test Case 3: Valid US Phone Number
    const char* validPhone = "(123) 456-7890";
    bool isPhoneValid = validateInput(validPhone, phoneRegex);
    printf("3. Is '%s' a valid US phone number? %s\n", validPhone, isPhoneValid ? "true" : "false");

    // Test Case 4: Valid Alphanumeric Username
    const char* validUsername = "user123";
    bool isUsernameValid = validateInput(validUsername, usernameRegex);
    printf("4. Is '%s' a valid username? %s\n", validUsername, isUsernameValid ? "true" : "false");

    // Test Case 5: Invalid Username (too short)
    const char* invalidUsername = "u1";
    bool isUsernameInvalid = validateInput(invalidUsername, usernameRegex);
    printf("5. Is '%s' a valid username? %s\n", invalidUsername, isUsernameInvalid ? "true" : "false");

    return 0;
}