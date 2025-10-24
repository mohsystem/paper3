#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <stdbool.h>

/**
 * Validates an input string against a given POSIX regular expression.
 *
 * @param inputString The string to validate.
 * @param regexPattern The regular expression pattern.
 * @return true if the input string matches the entire pattern, false otherwise.
 * @note This function uses POSIX regular expressions. It is assumed the regexPattern
 *       is from a trusted source to prevent ReDoS attacks. The pattern must be
 *       anchored with ^ and $ for full-string validation.
 */
bool validateInput(const char* inputString, const char* regexPattern) {
    if (inputString == NULL || regexPattern == NULL) {
        return false;
    }

    regex_t regex;
    int reti;
    bool is_match = false;

    // Compile the regular expression
    reti = regcomp(&regex, regexPattern, REG_EXTENDED);
    if (reti) {
        fprintf(stderr, "Could not compile regex\n");
        return false;
    }

    // Execute the regular expression
    reti = regexec(&regex, inputString, 0, NULL, 0);
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

    // Free the compiled regular expression
    regfree(&regex);

    return is_match;
}

int main() {
    // Test Case 1: Valid Email
    const char* emailValid = "test@example.com";
    // POSIX EREs often use character classes like [[:alnum:]] but \w is supported by many implementations.
    // Escaping backslashes is not needed in C string literals for non-special characters.
    const char* emailRegex = "^[a-zA-Z0-9._-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
    printf("1. Testing '%s' against email regex: %s\n", emailValid, validateInput(emailValid, emailRegex) ? "Valid" : "Invalid");

    // Test Case 2: Invalid Email
    const char* emailInvalid = "test@example";
    printf("2. Testing '%s' against email regex: %s\n", emailInvalid, validateInput(emailInvalid, emailRegex) ? "Valid" : "Invalid");

    // Test Case 3: Valid US Phone Number
    const char* phoneValid = "123-456-7890";
    const char* phoneRegex = "^[0-9]{3}-[0-9]{3}-[0-9]{4}$";
    printf("3. Testing '%s' against phone regex: %s\n", phoneValid, validateInput(phoneValid, phoneRegex) ? "Valid" : "Invalid");

    // Test Case 4: Valid Password (alphanumeric, 8+ characters)
    const char* passValid = "Password123";
    const char* passRegex = "^[A-Za-z0-9]{8,}$";
    printf("4. Testing '%s' against password regex: %s\n", passValid, validateInput(passValid, passRegex) ? "Valid" : "Invalid");

    // Test Case 5: Invalid Password (too short)
    const char* passInvalid = "pass";
    printf("5. Testing '%s' against password regex: %s\n", passInvalid, validateInput(passInvalid, passRegex) ? "Valid" : "Invalid");

    return 0;
}