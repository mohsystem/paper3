#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_USERS 10
#define MAX_LEN 128 // Max length for strings
#define HASH_LEN 65 // SHA-256 in hex is 64 chars + null terminator

typedef struct {
    char username[MAX_LEN];
    // In a real application, never store passwords. Store a strong, salted hash.
    char hashedPassword[HASH_LEN];
    char salt[MAX_LEN];
    char email[MAX_LEN];
} User;

// In-memory user database and session state
static User users[MAX_USERS];
static int user_count = 0;
static User* current_user = NULL;

/**
 * Hashes a password with a given salt.
 * WARNING: This is NOT a secure hashing function. It is for demonstration only.
 * It performs a simple XOR and hex conversion. For a real-world application,
 * use a dedicated, strong cryptographic library like libsodium or OpenSSL
 * to implement a function like Argon2, scrypt, or bcrypt.
 */
void hashPassword(const char* password, const char* salt, char* outputBuffer) {
    char temp[HASH_LEN] = {0};
    int pass_len = strlen(password);
    int salt_len = strlen(salt);
    
    for (int i = 0; i < HASH_LEN - 1; ++i) {
        char p_char = (i < pass_len) ? password[i] : ' ';
        char s_char = (i < salt_len) ? salt[i % salt_len] : ' ';
        temp[i] = p_char ^ s_char;
    }

    // Convert to hex string to make it look like a hash
    for(int i = 0; i < (HASH_LEN-1)/2; i++) {
        sprintf(outputBuffer + i*2, "%02x", (unsigned char)temp[i]);
    }
    outputBuffer[HASH_LEN-1] = '\0';
}

// Helper to find a user by username
User* findUser(const char* username) {
    for (int i = 0; i < user_count; ++i) {
        if (strncmp(users[i].username, username, MAX_LEN) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

bool registerUser(const char* username, const char* password, const char* email) {
    if (user_count >= MAX_USERS || findUser(username) != NULL) {
        return false;
    }
    if (strlen(username) >= MAX_LEN || strlen(password) >= MAX_LEN || strlen(email) >= MAX_LEN) {
        return false; // Input too long
    }

    User* newUser = &users[user_count];
    strncpy(newUser->username, username, MAX_LEN - 1);
    newUser->username[MAX_LEN-1] = '\0';
    
    strncpy(newUser->email, email, MAX_LEN - 1);
    newUser->email[MAX_LEN-1] = '\0';

    // Create a simple salt from username. In a real app, use a secure random source.
    snprintf(newUser->salt, MAX_LEN, "%s_s@lt", username);
    
    hashPassword(password, newUser->salt, newUser->hashedPassword);
    
    user_count++;
    return true;
}

bool login(const char* username, const char* password) {
    User* user = findUser(username);
    if (user != NULL) {
        char providedPasswordHash[HASH_LEN];
        hashPassword(password, user->salt, providedPasswordHash);
        
        // In a real application, use a constant-time comparison function
        // to prevent timing attacks. strncmp is not constant-time.
        if (strncmp(providedPasswordHash, user->hashedPassword, HASH_LEN) == 0) {
            current_user = user;
            return true;
        }
    }
    return false;
}

void logout() {
    current_user = NULL;
}

bool changeEmail(const char* oldEmail, const char* password, const char* newEmail) {
    if (current_user == NULL) {
        return false; // Not logged in
    }
    if (strlen(newEmail) >= MAX_LEN) {
        return false; // New email too long
    }

    if (strncmp(current_user->email, oldEmail, MAX_LEN) != 0) {
        return false;
    }

    char providedPasswordHash[HASH_LEN];
    hashPassword(password, current_user->salt, providedPasswordHash);
    
    if (strncmp(providedPasswordHash, current_user->hashedPassword, HASH_LEN) != 0) {
        return false;
    }

    // All checks passed
    strncpy(current_user->email, newEmail, MAX_LEN - 1);
    current_user->email[MAX_LEN-1] = '\0';
    return true;
}

int main() {
    const char* testUser = "testuser";
    const char* testPass = "Password123!";
    const char* testEmail = "test@example.com";

    printf("Registering user: %s\n", testUser);
    registerUser(testUser, testPass, testEmail);

    // Test Case 1: Successful login and email change.
    printf("\n--- Test Case 1: Successful Change ---\n");
    bool loggedIn = login(testUser, testPass);
    printf("Login successful: %s\n", loggedIn ? "true" : "false");
    if (loggedIn) {
        bool emailChanged = changeEmail(testEmail, testPass, "new@example.com");
        printf("Email change successful: %s\n", emailChanged ? "true" : "false");
        printf("User's new email: %s\n", current_user->email);
    }
    logout();

    // Test Case 2: Attempt to change email with the wrong password.
    printf("\n--- Test Case 2: Wrong Password ---\n");
    loggedIn = login(testUser, testPass);
    printf("Login successful: %s\n", loggedIn ? "true" : "false");
    if (loggedIn) {
        bool emailChanged = changeEmail("new@example.com", "WrongPassword!", "another@example.com");
        printf("Email change successful: %s\n", emailChanged ? "true" : "false");
        printf("User's email remains: %s\n", current_user->email);
    }
    logout();

    // Test Case 3: Attempt to change email with the wrong old email.
    printf("\n--- Test Case 3: Wrong Old Email ---\n");
    loggedIn = login(testUser, testPass);
    printf("Login successful: %s\n", loggedIn ? "true" : "false");
    if (loggedIn) {
        bool emailChanged = changeEmail("wrong@example.com", testPass, "another@example.com");
        printf("Email change successful: %s\n", emailChanged ? "true" : "false");
        printf("User's email remains: %s\n", current_user->email);
    }
    logout();
    
    // Test Case 4: Attempt to change email when not logged in.
    printf("\n--- Test Case 4: Not Logged In ---\n");
    bool emailChanged = changeEmail("new@example.com", testPass, "another@example.com");
    printf("Email change successful: %s\n", emailChanged ? "true" : "false");

    // Test Case 5: Successful change after a failed attempt.
    printf("\n--- Test Case 5: Successful Change After Failed Attempt ---\n");
    loggedIn = login(testUser, testPass);
    printf("Login successful: %s\n", loggedIn ? "true" : "false");
    if (loggedIn) {
        bool failedChange = changeEmail("new@example.com", "badpass", "final@example.com");
        printf("First change attempt (should fail): %s\n", failedChange ? "true" : "false");
        bool successChange = changeEmail("new@example.com", testPass, "final@example.com");
        printf("Second change attempt (should succeed): %s\n", successChange ? "true" : "false");
        printf("User's final email: %s\n", current_user->email);
    }
    logout();

    return 0;
}