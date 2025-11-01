#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// In a real application, these would not be hardcoded.
// The password would be a salted hash, not plaintext.
const char* CORRECT_USERNAME = "admin";
const char* CORRECT_PASSWORD = "P@ssw0rd!123";

/**
 * @brief Performs a constant-time string comparison to mitigate timing attacks.
 * 
 * @param a The first string.
 * @param b The second string.
 * @return true if the strings are equal, false otherwise.
 */
bool secure_equals(const char* a, const char* b) {
    if (a == NULL || b == NULL) {
        return a == b;
    }
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    // If lengths are different, they can't be equal.
    if (len_a != len_b) {
        return false;
    }

    // Use a variable to accumulate differences without short-circuiting.
    // The XOR result will be 0 only if all characters are the same.
    int diff = 0;
    for (size_t i = 0; i < len_a; ++i) {
        diff |= a[i] ^ b[i];
    }

    return diff == 0;
}

/**
 * @brief Securely checks if the provided username and password are correct.
 *
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return true if credentials are correct, false otherwise.
 */
bool check_mod(const char* username, const char* password) {
    bool userMatch = secure_equals(CORRECT_USERNAME, username);
    bool passMatch = secure_equals(CORRECT_PASSWORD, password);
    return userMatch && passMatch;
}

int main() {
    printf("Test Case 1: Correct credentials\n");
    printf("Input: admin, P@ssw0rd!123\n");
    printf("Output: %s\n", check_mod("admin", "P@ssw0rd!123") ? "true" : "false"); // Expected: true
    printf("--------------------\n");

    printf("Test Case 2: Correct username, incorrect password\n");
    printf("Input: admin, wrongpassword\n");
    printf("Output: %s\n", check_mod("admin", "wrongpassword") ? "true" : "false"); // Expected: false
    printf("--------------------\n");

    printf("Test Case 3: Incorrect username, correct password\n");
    printf("Input: user, P@ssw0rd!123\n");
    printf("Output: %s\n", check_mod("user", "P@ssw0rd!123") ? "true" : "false"); // Expected: false
    printf("--------------------\n");

    printf("Test Case 4: Incorrect credentials\n");
    printf("Input: user, password\n");
    printf("Output: %s\n", check_mod("user", "password") ? "true" : "false"); // Expected: false
    printf("--------------------\n");

    printf("Test Case 5: Correct username, password with different length\n");
    printf("Input: admin, P@ss\n");
    printf("Output: %s\n", check_mod("admin", "P@ss") ? "true" : "false"); // Expected: false
    printf("--------------------\n");

    return 0;
}