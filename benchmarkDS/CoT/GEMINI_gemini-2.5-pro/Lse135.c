#include <stdio.h>
#include <string.h>
#include <stdbool.h> // For bool type, true, and false

// NOTE: Storing credentials in plaintext is a major security risk.
// In a real application, use a secure method like hashed and salted passwords.
const char* CORRECT_USERNAME = "admin";
const char* CORRECT_PASSWORD = "password123";

/**
 * Performs a constant-time string comparison to mitigate timing attacks.
 *
 * @param a The first string.
 * @param b The second string.
 * @return true if strings are equal, false otherwise.
 */
bool secure_compare(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return a == b;
    }
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    if (len_a != len_b) {
        return false;
    }
    
    // Use a bitwise OR operation that does not short-circuit.
    // This ensures the comparison takes the same amount of time regardless of where the first difference occurs.
    int result = 0;
    for (size_t i = 0; i < len_a; ++i) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

/**
 * Checks if the provided username and password are correct.
 *
 * @param username The username to check.
 * @param password The password to check.
 * @return true if credentials are correct, false otherwise.
 */
bool check_mod(const char* username, const char* password) {
    if (username == NULL || password == NULL) {
        return false;
    }

    // strcmp is vulnerable to timing attacks, but it is less critical for usernames.
    // For consistency and higher security, we can use secure_compare for both.
    bool isUsernameCorrect = (strcmp(username, CORRECT_USERNAME) == 0);
    
    // Use a constant-time comparison for the password.
    bool isPasswordCorrect = secure_compare(password, CORRECT_PASSWORD);

    return isUsernameCorrect && isPasswordCorrect;
}

int main() {
    // Helper to print bools as "true" or "false"
    #define PRINT_BOOL(b) ((b) ? "true" : "false")

    // Test Case 1: Correct username and password
    printf("Test Case 1 (Correct credentials): %s\n", PRINT_BOOL(check_mod("admin", "password123")));

    // Test Case 2: Correct username, incorrect password
    printf("Test Case 2 (Incorrect password): %s\n", PRINT_BOOL(check_mod("admin", "wrongpassword")));

    // Test Case 3: Incorrect username, correct password
    printf("Test Case 3 (Incorrect username): %s\n", PRINT_BOOL(check_mod("user", "password123")));

    // Test Case 4: Incorrect username and password
    printf("Test Case 4 (Incorrect credentials): %s\n", PRINT_BOOL(check_mod("user", "wrongpassword")));

    // Test Case 5: Null inputs
    printf("Test Case 5 (Null input): %s\n", PRINT_BOOL(check_mod("admin", NULL)));

    return 0;
}