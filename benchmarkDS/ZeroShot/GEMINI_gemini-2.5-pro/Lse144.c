#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Mock database of subscribed emails. In a real application, this would be a database call.
const char* emailDatabase[] = {
    "test@example.com",
    "user1@test.com",
    "fail@example.com",
    "subscriber@newsletter.org",
    NULL // Sentinel value to mark the end of the array
};

/**
 * @brief Simulates checking if an email exists in the database.
 * @param email The email to check.
 * @return true if the email exists, false otherwise.
 */
bool emailExists(const char* email) {
    // SECURITY: In a real application, use a parameterized query or a prepared
    // statement to prevent SQL injection when querying the database.
    for (int i = 0; emailDatabase[i] != NULL; i++) {
        if (strcmp(emailDatabase[i], email) == 0) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Simulates running an external script to unsubscribe an email.
 * @param email The email to unsubscribe.
 * @return 0 on success, a non-zero value on failure.
 */
int runUnsubscribeScript(const char* email) {
    // SECURITY: Never build a command string by concatenating user input.
    // This prevents command injection vulnerabilities. Use APIs like execvp() 
    // where arguments are passed in an array, not a single parsed string.
    printf("   -> Running unsubscribe script for: %s\n", email);

    // Mock logic: script fails if the email contains "fail".
    if (strstr(email, "fail") != NULL) {
        printf("   -> Script failed.\n");
        return -1; // Non-zero indicates failure
    }
    
    printf("   -> Script succeeded.\n");
    return 0; // 0 indicates success
}

/**
 * @brief Performs basic validation on an email string.
 * A full regex implementation is non-trivial in standard C. This is a simplification.
 * @param email The email string to validate.
 * @return true if the format seems plausible, false otherwise.
 */
bool isValidEmailFormat(const char* email) {
    if (email == NULL) return false;
    const char* at_symbol = strchr(email, '@');
    if (at_symbol == NULL || at_symbol == email) return false; // Must have '@' not at the start
    
    const char* dot_symbol = strrchr(at_symbol, '.');
    // Must have '.' after '@', and not immediately after.
    if (dot_symbol == NULL || dot_symbol == at_symbol + 1) return false;
    
    // Ensure the dot is not the last character
    if (dot_symbol[1] == '\0') return false;

    return true;
}

/**
 * @brief Checks if an email exists and unsubscribes it.
 * @param email The email address to process.
 * @return 1 if the email existed and the unsubscribe script succeeded.
 *         0 in all other cases (email not found, invalid email, or script failure).
 */
int unsubscribeEmail(const char* email) {
    // 1. Input Validation
    if (!isValidEmailFormat(email)) {
        printf("Input '%s' is invalid.\n", email ? email : "NULL");
        return 0;
    }

    // 2. Check if email exists in the database.
    if (!emailExists(email)) {
        printf("Email '%s' not found in database.\n", email);
        return 0;
    }

    printf("Email '%s' found. Attempting to unsubscribe.\n", email);
    // 3. Call the unsubscribe script.
    int scriptResult = runUnsubscribeScript(email);

    // 4. Check script result and return final status.
    if (scriptResult == 0) {
        return 1; // Script succeeded
    } else {
        return 0; // Script failed
    }
}

int main() {
    const char* testEmails[] = {
        "test@example.com",       // Exists, script will succeed
        "fail@example.com",       // Exists, script will fail
        "notfound@example.com",   // Does not exist
        "invalid-email",          // Invalid format
        "subscriber@newsletter.org"// Exists, script will succeed
    };

    int expectedResults[] = {1, 0, 0, 0, 1};
    int numTests = sizeof(testEmails) / sizeof(testEmails[0]);

    for (int i = 0; i < numTests; i++) {
        printf("--- Test Case %d: %s ---\n", i + 1, testEmails[i]);
        int result = unsubscribeEmail(testEmails[i]);
        printf("Result: %d (Expected: %d)\n", result, expectedResults[i]);
        printf("Status: %s\n\n", (result == expectedResults[i]) ? "PASSED" : "FAILED");
    }

    return 0;
}