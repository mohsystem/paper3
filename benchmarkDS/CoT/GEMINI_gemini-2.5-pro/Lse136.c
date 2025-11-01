#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// NOTE: Storing credentials in plaintext is a major security risk.
// In a real application, use a strong, salted hashing algorithm (e.g., Argon2, bcrypt)
// to store password hashes and compare against the hash of the user-provided password.
// These are hardcoded for demonstration purposes only.
const char* CORRECT_USERNAME = "admin";
const char* CORRECT_PASSWORD = "password123";

/**
 * Performs a constant-time string comparison to mitigate timing attacks.
 *
 * @param s1 The first string.
 * @param s2 The second string.
 * @return true if the strings are equal, false otherwise.
 */
bool secure_strcmp(const char* s1, const char* s2) {
    if (s1 == NULL || s2 == NULL) {
        return s1 == s2;
    }
    
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);

    if (len1 != len2) {
        return false;
    }

    int diff = 0;
    for (size_t i = 0; i < len1; ++i) {
        diff |= s1[i] ^ s2[i];
    }
    return diff == 0;
}


/**
 * Simulates checking a username and password from a URL.
 * Returns "true" if credentials are correct, "false" otherwise.
 *
 * @param username The username to check.
 * @param password The password to check.
 * @return The string "true" on success, "false" on failure.
 */
const char* check_mod(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return "false";
    }

    // Usernames are often not secret, so a standard comparison is acceptable.
    bool isUsernameCorrect = (strcmp(CORRECT_USERNAME, username) == 0);
    
    // Passwords must be compared in constant time to prevent timing attacks.
    bool isPasswordCorrect = secure_strcmp(CORRECT_PASSWORD, password);

    if (isUsernameCorrect && isPasswordCorrect) {
        return "true";
    } else {
        return "false";
    }
}

int main() {
    // Test cases
    const char* testUsernames[] = {"admin", "admin", "user", "user", ""};
    const char* testPasswords[] = {"password123", "wrongpassword", "password123", "wrongpassword", ""};
    int numTestCases = 5;

    printf("Running test cases...\n");
    for (int i = 0; i < numTestCases; i++) {
        const char* username = testUsernames[i];
        const char* password = testPasswords[i];
        const char* result = check_mod(username, password);
        printf("Test Case %d:\n", i + 1);
        printf("  Username: \"%s\"\n", username);
        printf("  Password: \"%s\"\n", password);
        printf("  Result: %s\n\n", result);
    }

    return 0;
}