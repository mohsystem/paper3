#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// WARNING: This is a simplified example for demonstration purposes only.
// DO NOT use hardcoded credentials or plaintext password comparison in production applications.
// In a real-world scenario, passwords should be securely stored as salted hashes
// using a strong algorithm like Argon2, scrypt, or PBKDF2, and credentials should
// be managed through a secure configuration or secrets management system.
const char* CORRECT_USERNAME = "admin";
const char* CORRECT_PASSWORD = "Pa$$w0rd123!";

/**
 * @brief Performs a constant-time comparison of two C-style strings.
 * This is crucial for comparing secrets like passwords to mitigate timing attacks.
 * @param a The first string.
 * @param b The second string.
 * @return true if the strings are equal, false otherwise.
 */
bool constant_time_compare(const char* a, const char* b) {
    // Rule#1: Validate input.
    if (a == NULL || b == NULL) {
        return a == b; // Return true only if both are NULL.
    }
    
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);

    if (len_a != len_b) {
        return false;
    }
    
    int result = 0;
    for (size_t i = 0; i < len_a; ++i) {
        result |= a[i] ^ b[i];
    }
    
    return result == 0;
}

/**
 * @brief Checks if the provided username and password are correct.
 * This function simulates checking credentials from a form.
 * @param username The username to check.
 * @param password The password to check.
 * @return true if the credentials are correct, false otherwise.
 */
bool check_mod(const char* username, const char* password) {
    // Rule#1: Ensure all input is validated.
    if (username == NULL || password == NULL) {
        return false;
    }

    // Username can be compared directly using strcmp.
    bool usernameMatch = (strcmp(CORRECT_USERNAME, username) == 0);

    // Use a constant-time comparison for the password to mitigate timing attacks.
    bool passwordMatch = constant_time_compare(CORRECT_PASSWORD, password);

    return usernameMatch && passwordMatch;
}

int main() {
    // 5 Test Cases
    printf("Test Case 1: Correct credentials\n");
    printf("Input: user='admin', pass='Pa$$w0rd123!'\n");
    printf("Result: %s\n", check_mod("admin", "Pa$$w0rd123!") ? "true" : "false"); // Expected: true

    printf("\nTest Case 2: Incorrect username\n");
    printf("Input: user='user', pass='Pa$$w0rd123!'\n");
    printf("Result: %s\n", check_mod("user", "Pa$$w0rd123!") ? "true" : "false"); // Expected: false

    printf("\nTest Case 3: Incorrect password\n");
    printf("Input: user='admin', pass='wrongpassword'\n");
    printf("Result: %s\n", check_mod("admin", "wrongpassword") ? "true" : "false"); // Expected: false
    
    printf("\nTest Case 4: Null input\n");
    printf("Input: user=NULL, pass='Pa$$w0rd123!'\n");
    printf("Result: %s\n", check_mod(NULL, "Pa$$w0rd123!") ? "true" : "false"); // Expected: false

    printf("\nTest Case 5: Empty inputs\n");
    printf("Input: user='', pass=''\n");
    printf("Result: %s\n", check_mod("", "") ? "true" : "false"); // Expected: false

    return 0;
}