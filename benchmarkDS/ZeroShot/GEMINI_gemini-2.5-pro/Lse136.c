#include <stdio.h>
#include <string.h>

// WARNING: Storing credentials in plaintext is highly insecure.
// In a real application, use a secure password hashing mechanism (e.g., Argon2, bcrypt)
// and store the hashes in a secure database. This is for demonstration only.
const char* CORRECT_USERNAME = "admin";
const char* CORRECT_PASSWORD = "supersecretpassword123!";

/**
 * Performs a constant-time comparison of two C strings to prevent timing attacks.
 * It compares every byte and only reveals the result at the very end.
 *
 * @param a First string.
 * @param b Second string.
 * @return 1 if strings are equal, 0 otherwise.
 */
int secure_compare(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return 0; // Not equal if one or both are null
    }

    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    // Mismatched lengths are an immediate fail, but we must not return early.
    // We XOR the lengths and use this as the initial difference.
    unsigned char diff = (unsigned char)(len_a ^ len_b);

    // We iterate up to the length of the shorter string.
    size_t min_len = (len_a < len_b) ? len_a : len_b;
    for (size_t i = 0; i < min_len; ++i) {
        // Accumulate differences using bitwise OR.
        diff |= (unsigned char)a[i] ^ (unsigned char)b[i];
    }
    
    // The result is true (1) only if the accumulated difference is 0.
    return (diff == 0);
}


/**
 * Checks if the provided username and password are correct.
 * This function is designed to be called when a user visits a specific URL.
 * It uses a constant-time comparison to mitigate timing attacks.
 *
 * @param username The username obtained from the URL.
 * @param password The password obtained from the URL.
 * @return The string "true" if the credentials are correct, "false" otherwise.
 */
const char* check_mod(const char* username, const char* password) {
    // Use the secure comparison for both username and password.
    int is_username_match = secure_compare(CORRECT_USERNAME, username);
    int is_password_match = secure_compare(CORRECT_PASSWORD, password);

    // Use a bitwise AND (&) to ensure both comparisons are executed before the check.
    if (is_username_match & is_password_match) {
        return "true";
    } else {
        return "false";
    }
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Correct credentials
    printf("Test 1 (Correct): %s\n", check_mod("admin", "supersecretpassword123!"));

    // Test Case 2: Correct username, incorrect password
    printf("Test 2 (Wrong Pass): %s\n", check_mod("admin", "wrongpassword"));

    // Test Case 3: Incorrect username, correct password
    printf("Test 3 (Wrong User): %s\n", check_mod("user", "supersecretpassword123!"));

    // Test Case 4: Incorrect credentials
    printf("Test 4 (All Wrong): %s\n", check_mod("user", "wrongpassword"));

    // Test Case 5: Null inputs
    printf("Test 5 (Null Input): %s\n", check_mod(NULL, "somepassword"));
    
    return 0;
}