#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_USERNAME_LEN 50
#define MAX_EMAIL_LEN 100
#define MAX_HASH_LEN 100
#define MAX_PASSWORD_LEN 50
#define MAX_USERS 10

// --- Mock User Data Structure and Database ---
typedef struct {
    char username[MAX_USERNAME_LEN];
    char email[MAX_EMAIL_LEN];
    char passwordHash[MAX_HASH_LEN];
} User;

// In a real application, this would be a database.
User userDatabase[MAX_USERS];
int userCount = 0;

// In a real application, session management would be more robust.
User* currentUser = NULL;

// --- Security Utilities (Mocks) ---

// MOCK HASHING: NOT FOR PRODUCTION USE.
// Use a well-vetted cryptography library (like OpenSSL or libsodium)
// for Argon2/BCrypt in a real application.
void hashPassword(const char* password, char* hash_buffer) {
    // This is a simple, insecure "hash" for demonstration only.
    strncpy(hash_buffer, "hashed_", MAX_HASH_LEN -1);
    size_t current_len = strlen(hash_buffer);
    for (size_t i = 0; i < strlen(password) && current_len < MAX_HASH_LEN - 2; ++i) {
        hash_buffer[current_len++] = (password[i] % 10) + '0';
    }
    hash_buffer[current_len] = '\0';
}

// MOCK VERIFICATION: NOT FOR PRODUCTION USE.
// This comparison should be done in a constant-time manner.
bool verifyPassword(const char* plainPassword, const char* storedHash) {
    if (storedHash == NULL || plainPassword == NULL) {
        return false;
    }
    char newHash[MAX_HASH_LEN];
    hashPassword(plainPassword, newHash);
    return strcmp(newHash, storedHash) == 0;
}

// Basic email format validation
bool isValidEmailFormat(const char* email) {
    if (email == NULL) return false;
    // A simple check, a more robust check would be used in production.
    return strchr(email, '@') != NULL && strchr(email, '.') != NULL;
}

// --- Core Functionality ---
void addUser(const char* username, const char* email, const char* password) {
    if (userCount < MAX_USERS) {
        strncpy(userDatabase[userCount].username, username, MAX_USERNAME_LEN - 1);
        strncpy(userDatabase[userCount].email, email, MAX_EMAIL_LEN - 1);
        hashPassword(password, userDatabase[userCount].passwordHash);
        userCount++;
    }
}

bool login(const char* username, const char* password) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            if (verifyPassword(password, userDatabase[i].passwordHash)) {
                currentUser = &userDatabase[i];
                return true;
            }
        }
    }
    currentUser = NULL;
    return false;
}

void logout() {
    currentUser = NULL;
}

const char* changeEmail(const char* oldEmail, const char* newEmail, const char* password) {
    // 1. Check if a user is logged in
    if (currentUser == NULL) {
        return "Error: You must be logged in to change your email.";
    }

    // 2. Verify password
    if (!verifyPassword(password, currentUser->passwordHash)) {
        return "Error: Incorrect password.";
    }

    // 3. Verify old email
    if (strcmp(currentUser->email, oldEmail) != 0) {
        return "Error: The old email address does not match our records.";
    }

    // 4. Check if new email is same as old
    if (strcmp(oldEmail, newEmail) == 0) {
        return "Error: New email cannot be the same as the old email.";
    }

    // 5. Validate new email format
    if (!isValidEmailFormat(newEmail)) {
        return "Error: Invalid new email format.";
    }

    // All checks passed, update email. Use strncpy for safety.
    strncpy(currentUser->email, newEmail, MAX_EMAIL_LEN - 1);
    currentUser->email[MAX_EMAIL_LEN - 1] = '\0'; // Ensure null termination
    
    // In C, returning a formatted string requires a static or allocated buffer.
    // Static is easier for this example but not thread-safe.
    static char success_message[MAX_EMAIL_LEN + 50];
    snprintf(success_message, sizeof(success_message), "Success: Email has been updated to %s", newEmail);
    return success_message;
}

// --- Main function with test cases ---
int main() {
    // Setup: Create a user
    const char* username = "testuser";
    const char* initial_email = "user@example.com";
    const char* initial_password = "Password123!";
    addUser(username, initial_email, initial_password);
    
    printf("--- Test Case 1: Successful Email Change ---\n");
    login(username, initial_password);
    printf("Login successful for: %s\n", (currentUser ? currentUser->username : "none"));
    const char* result1 = changeEmail(initial_email, "new.user@example.com", initial_password);
    printf("Result: %s\n", result1);
    printf("Current email: %s\n", (currentUser ? currentUser->email : "N/A"));
    logout();
    printf("\n-------------------------------------------------\n\n");

    // Reset user email for next tests
    strncpy(userDatabase[0].email, initial_email, MAX_EMAIL_LEN);

    printf("--- Test Case 2: Failure - Not Logged In ---\n");
    printf("Current user is logged out.\n");
    const char* result2 = changeEmail(initial_email, "another.email@example.com", initial_password);
    printf("Result: %s\n", result2);
    printf("\n-------------------------------------------------\n\n");

    printf("--- Test Case 3: Failure - Incorrect Password ---\n");
    login(username, initial_password);
    printf("Login successful for: %s\n", (currentUser ? currentUser->username : "none"));
    const char* result3 = changeEmail(initial_email, "another.email@example.com", "WrongPassword!");
    printf("Result: %s\n", result3);
    printf("Email was not changed: %s\n", currentUser->email);
    logout();
    printf("\n-------------------------------------------------\n\n");

    printf("--- Test Case 4: Failure - Incorrect Old Email ---\n");
    login(username, initial_password);
    printf("Login successful for: %s\n", (currentUser ? currentUser->username : "none"));
    const char* result4 = changeEmail("wrong.old@example.com", "another.email@example.com", initial_password);
    printf("Result: %s\n", result4);
    printf("Email was not changed: %s\n", currentUser->email);
    logout();
    printf("\n-------------------------------------------------\n\n");

    printf("--- Test Case 5: Failure - New Email is Same as Old ---\n");
    login(username, initial_password);
    printf("Login successful for: %s\n", (currentUser ? currentUser->username : "none"));
    const char* result5 = changeEmail(initial_email, initial_email, initial_password);
    printf("Result: %s\n", result5);
    printf("Email was not changed: %s\n", currentUser->email);
    logout();
    printf("\n-------------------------------------------------\n\n");

    return 0;
}