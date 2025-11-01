#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// NOTE: Storing credentials directly in the code is highly insecure.
// In a real application, use a secure vault, environment variables,
// and store password hashes (e.g., using Argon2) instead of plaintext.
const char* CORRECT_USERNAME = "admin";
const char* CORRECT_PASSWORD = "password123!";
#define MAX_INPUT_LENGTH 128

/**
 * @brief Performs a constant-time string comparison to mitigate timing attacks.
 * @note This is an illustrative implementation. A production system should use a
 * well-vetted cryptographic library function. This simple version may leak length information.
 * @param user_input The first string.
 * @param stored_secret The second string.
 * @return True if the strings are equal, false otherwise.
 */
bool secure_compare(const char* user_input, const char* stored_secret) {
    size_t user_len = strlen(user_input);
    size_t stored_len = strlen(stored_secret);
    
    // Length check must be done, but reveals length information.
    // A more robust solution might involve hashing both inputs before comparing.
    if (user_len != stored_len) {
        return false;
    }

    int diff = 0;
    // Compare byte by byte without short-circuiting.
    for (size_t i = 0; i < stored_len; ++i) {
        diff |= user_input[i] ^ stored_secret[i];
    }

    return diff == 0;
}

/**
 * @brief Validates user credentials in a secure manner.
 * @param username The username provided by the user.
 * @param password The password provided by the user.
 * @return True if login is successful, false otherwise.
 */
bool login(const char* username, const char* password) {
    // Rule #1: Ensure all input is validated.
    if (username == NULL || username[0] == '\0') {
        fprintf(stderr, "Invalid username format.\n");
        return false;
    }
    // Rule #3: Use boundary checks to prevent buffer over-reads.
    if (strnlen(username, MAX_INPUT_LENGTH + 1) > MAX_INPUT_LENGTH) {
        fprintf(stderr, "Invalid username format: too long.\n");
        return false;
    }
    
    if (password == NULL || password[0] == '\0') {
        fprintf(stderr, "Invalid password format.\n");
        return false;
    }
    if (strnlen(password, MAX_INPUT_LENGTH + 1) > MAX_INPUT_LENGTH) {
        fprintf(stderr, "Invalid password format: too long.\n");
        return false;
    }

    // Standard comparison for username (not a secret). Use strcmp.
    bool username_matches = (strcmp(username, CORRECT_USERNAME) == 0);
    
    // Rule #7 & #8: Use constant-time comparison for secrets.
    bool password_matches = secure_compare(password, CORRECT_PASSWORD);

    if (username_matches && password_matches) {
        printf("Login successful! Redirecting to session page...\n");
        return true;
    } else {
        printf("Incorrect username or password.\n");
        return false;
    }
}

int main() {
    printf("--- Test Case 1: Correct credentials ---\n");
    login("admin", "password123!");

    printf("\n--- Test Case 2: Incorrect password ---\n");
    login("admin", "wrongpassword");

    printf("\n--- Test Case 3: Incorrect username ---\n");
    login("user", "password123!");

    printf("\n--- Test Case 4: NULL input ---\n");
    login(NULL, "password123!");
    
    printf("\n--- Test Case 5: Empty input ---\n");
    login("admin", "");
    
    return 0;
}