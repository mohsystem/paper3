#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Define buffer sizes
#define MAX_EMAIL_LEN 100
#define MAX_PASSWORD_LEN 50

// Simulate a single user's session data
char currentUserEmail[MAX_EMAIL_LEN];
char currentUserPassword[MAX_PASSWORD_LEN];
bool isLoggedIn = false;

/**
 * @brief Helper to set up an initial user for testing.
 * @param email The user's email.
 * @param password The user's password.
 */
void registerUser(const char* email, const char* password) {
    strncpy(currentUserEmail, email, MAX_EMAIL_LEN - 1);
    currentUserEmail[MAX_EMAIL_LEN - 1] = '\0'; // Ensure null termination
    strncpy(currentUserPassword, password, MAX_PASSWORD_LEN - 1);
    currentUserPassword[MAX_PASSWORD_LEN - 1] = '\0'; // Ensure null termination
    isLoggedIn = false; // User is registered, not logged in
}

/**
 * @brief Logs a user in.
 * @param email The email to log in with.
 * @param password The password for the account.
 * @return true if login is successful, false otherwise.
 */
bool login(const char* email, const char* password) {
    if (strlen(currentUserEmail) > 0 && strcmp(currentUserEmail, email) == 0 &&
        strlen(currentUserPassword) > 0 && strcmp(currentUserPassword, password) == 0) {
        isLoggedIn = true;
        return true;
    }
    isLoggedIn = false;
    return false;
}

/**
 * @brief Logs the current user out.
 */
void logout() {
    isLoggedIn = false;
}

/**
 * @brief Changes the current user's email after validation.
 * @param oldEmail The user's current email for confirmation.
 * @param newEmail The desired new email.
 * @param password The user's password for confirmation.
 * @return A string literal indicating success or the specific failure reason.
 */
const char* changeEmail(const char* oldEmail, const char* newEmail, const char* password) {
    if (!isLoggedIn) {
        return "Error: User is not logged in.";
    }
    if (strcmp(currentUserEmail, oldEmail) != 0) {
        return "Error: Old email does not match the current email.";
    }
    if (strcmp(currentUserPassword, password) != 0) {
        return "Error: Incorrect password.";
    }
    if (strcmp(oldEmail, newEmail) == 0) {
        return "Error: New email cannot be the same as the old email.";
    }

    // If all checks pass, change the email
    strncpy(currentUserEmail, newEmail, MAX_EMAIL_LEN - 1);
    currentUserEmail[MAX_EMAIL_LEN - 1] = '\0';
    
    return "Email changed successfully.";
}


int main() {
    // Initial setup
    const char* initialEmail = "user@example.com";
    const char* initialPassword = "password123";

    // --- Test Case 1: Successful Email Change ---
    printf("--- Test Case 1: Successful Change ---\n");
    registerUser(initialEmail, initialPassword);
    printf("User registered: %s\n", initialEmail);
    bool loginStatus = login(initialEmail, initialPassword);
    printf("Login status: %s\n", loginStatus ? "true" : "false");
    const char* result1 = changeEmail(initialEmail, "new.user@example.com", initialPassword);
    printf("Change email result: %s\n", result1);
    printf("Current email is now: %s\n\n", currentUserEmail);

    // --- Test Case 2: Failed Change (Incorrect Password) ---
    printf("--- Test Case 2: Incorrect Password ---\n");
    registerUser(initialEmail, initialPassword); // Reset state
    printf("User registered: %s\n", initialEmail);
    login(initialEmail, initialPassword);
    printf("Login status: %s\n", isLoggedIn ? "true" : "false");
    const char* result2 = changeEmail(initialEmail, "another.user@example.com", "wrongpassword");
    printf("Change email result: %s\n", result2);
    printf("Current email is still: %s\n\n", currentUserEmail);

    // --- Test Case 3: Failed Change (Not Logged In) ---
    printf("--- Test Case 3: Not Logged In ---\n");
    registerUser(initialEmail, initialPassword);
    printf("User registered: %s\n", initialEmail);
    logout(); // Ensure user is logged out
    printf("Login status: %s\n", isLoggedIn ? "true" : "false");
    const char* result3 = changeEmail(initialEmail, "new.user@example.com", initialPassword);
    printf("Change email result: %s\n\n", result3);

    // --- Test Case 4: Failed Change (New Email is Same as Old) ---
    printf("--- Test Case 4: New Email is Same as Old ---\n");
    registerUser(initialEmail, initialPassword);
    printf("User registered: %s\n", initialEmail);
    login(initialEmail, initialPassword);
    printf("Login status: %s\n", isLoggedIn ? "true" : "false");
    const char* result4 = changeEmail(initialEmail, initialEmail, initialPassword);
    printf("Change email result: %s\n\n", result4);

    // --- Test Case 5: Failed Change (Incorrect Old Email Provided) ---
    printf("--- Test Case 5: Incorrect Old Email Provided ---\n");
    registerUser(initialEmail, initialPassword);
    printf("User registered: %s\n", initialEmail);
    login(initialEmail, initialPassword);
    printf("Login status: %s\n", isLoggedIn ? "true" : "false");
    const char* result5 = changeEmail("wrong.old@example.com", "new.user@example.com", initialPassword);
    printf("Change email result: %s\n\n", result5);

    return 0;
}