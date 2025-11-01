#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>

// To compile: gcc your_file_name.c -o your_program -lssl -lcrypto
// This code requires OpenSSL development libraries to be installed.

#define MAX_USERNAME_LEN 50
#define MAX_EMAIL_LEN 100
#define MAX_PASSWORD_LEN 100
#define SALT_SIZE 16
#define HASH_SIZE 32
#define PBKDF2_ITERATIONS 260000
#define MAX_USERS 10

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    char email[MAX_EMAIL_LEN + 1];
    unsigned char passwordHash[HASH_SIZE];
    unsigned char salt[SALT_SIZE];
    bool isLoggedIn;
} User;

// In a real application, use a proper database. This is for demonstration.
User userDatabase[MAX_USERS];
int userCount = 0;

// Rule #15: Provides a way to securely zero memory that compilers won't optimize away.
void secure_zero_memory(void *v, size_t n) {
    volatile unsigned char *p = v;
    while (n--) {
        *p++ = 0;
    }
}

int generate_salt(unsigned char* salt, size_t size) {
    return RAND_bytes(salt, size);
}

int hash_password(const char* password, const unsigned char* salt, unsigned char* hash) {
    if (password == NULL || strlen(password) == 0) {
        return 0; // Failure
    }
    int result = PKCS5_PBKDF2_HMAC(
        password, strlen(password),
        salt, SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        HASH_SIZE,
        hash
    );
    return result;
}

bool verify_password(const char* password, const User* user) {
    if (password == NULL || strlen(password) == 0) {
        return false;
    }
    unsigned char newHash[HASH_SIZE];
    if (hash_password(password, user->salt, newHash) != 1) {
        return false;
    }
    // Rule #7: Constant-time comparison
    bool match = (CRYPTO_memcmp(newHash, user->passwordHash, HASH_SIZE) == 0);
    secure_zero_memory(newHash, HASH_SIZE);
    return match;
}

User* find_user(const char* username) {
    for (int i = 0; i < userCount; ++i) {
        if (strncmp(userDatabase[i].username, username, MAX_USERNAME_LEN) == 0) {
            return &userDatabase[i];
        }
    }
    return NULL;
}

void create_user(const char* username, const char* email, const char* password) {
    if (userCount >= MAX_USERS) {
        printf("Database is full.\n");
        return;
    }
    User* newUser = &userDatabase[userCount];

    strncpy(newUser->username, username, MAX_USERNAME_LEN);
    newUser->username[MAX_USERNAME_LEN] = '\0';
    
    strncpy(newUser->email, email, MAX_EMAIL_LEN);
    newUser->email[MAX_EMAIL_LEN] = '\0';
    
    if (generate_salt(newUser->salt, SALT_SIZE) != 1) {
        printf("Failed to generate salt.\n");
        return;
    }
    if (hash_password(password, newUser->salt, newUser->passwordHash) != 1) {
        printf("Failed to hash password.\n");
        return;
    }
    newUser->isLoggedIn = false;
    userCount++;
}

bool is_valid_email(const char* email) {
    const char *at = strchr(email, '@');
    const char *dot = strrchr(email, '.');
    // Very basic check: must contain '@' and '.' after '@'
    return at && dot && dot > at;
}

bool change_email(const char* username, const char* oldEmail, const char* newEmail, const char* password) {
    // Rule #1: Input validation
    if (!username || !oldEmail || !newEmail || !password ||
        strlen(username) == 0 || strlen(oldEmail) == 0 || strlen(newEmail) == 0 || strlen(password) == 0 ||
        strlen(username) > MAX_USERNAME_LEN || strlen(oldEmail) > MAX_EMAIL_LEN ||
        strlen(newEmail) > MAX_EMAIL_LEN || strlen(password) > MAX_PASSWORD_LEN) {
        printf("Error: Invalid or missing input.\n");
        return false;
    }
    
    if (!is_valid_email(newEmail)) {
        printf("Error: New email format is invalid.\n");
        return false;
    }

    User* user = find_user(username);

    if (user == NULL) {
        // Generic error to prevent user enumeration
        printf("Error: Email change failed.\n");
        return false;
    }

    // Security checks
    if (!user->isLoggedIn) {
        printf("Error: User must be logged in to change email.\n");
        return false;
    }

    if (strncmp(newEmail, oldEmail, MAX_EMAIL_LEN) == 0) {
        printf("Error: New email must be different from the old email.\n");
        return false;
    }

    if (strncmp(user->email, oldEmail, MAX_EMAIL_LEN) != 0) {
        printf("Error: Email change failed.\n"); // Generic error
        return false;
    }

    if (!verify_password(password, user)) {
        printf("Error: Email change failed.\n"); // Generic error
        return false;
    }
    
    // All checks passed, update email
    strncpy(user->email, newEmail, MAX_EMAIL_LEN);
    user->email[MAX_EMAIL_LEN] = '\0';

    printf("Email for user '%s' has been successfully updated to %s\n", username, newEmail);
    return true;
}


int main() {
    // Setup: Create a user
    const char* testUsername = "testUser";
    const char* initialEmail = "initial@example.com";
    const char* userPassword = "Password123!";

    create_user(testUsername, initialEmail, userPassword);
    User* testUser = find_user(testUsername);
    if (!testUser) {
        printf("Failed to create test user.\n");
        return 1;
    }

    printf("Initial state: User '%s' with email '%s' is logged out.\n", testUser->username, testUser->email);
    printf("--------------------------------------------------\n");

    // --- Test Cases ---
    printf("Running Test Cases...\n\n");

    // Test Case 1: Successful email change
    printf("--- Test Case 1: Successful Change ---\n");
    testUser->isLoggedIn = true; // Simulate login
    printf("User is now logged in.\n");
    bool result1 = change_email(testUsername, initialEmail, "new.email@example.com", userPassword);
    printf("Result: %s\n", result1 ? "SUCCESS" : "FAILURE");
    printf("Current email: %s\n\n", testUser->email);
    
    char updatedEmail[MAX_EMAIL_LEN + 1];
    strncpy(updatedEmail, testUser->email, MAX_EMAIL_LEN);
    updatedEmail[MAX_EMAIL_LEN] = '\0';

    // Test Case 2: Failure - User not logged in
    printf("--- Test Case 2: Failure (Not Logged In) ---\n");
    testUser->isLoggedIn = false; // Simulate logout
    printf("User is now logged out.\n");
    bool result2 = change_email(testUsername, updatedEmail, "another.email@example.com", userPassword);
    printf("Result: %s\n", result2 ? "SUCCESS" : "FAILURE");
    printf("Current email: %s\n\n", testUser->email);
    testUser->isLoggedIn = true; // Log back in for subsequent tests

    // Test Case 3: Failure - Incorrect password
    printf("--- Test Case 3: Failure (Incorrect Password) ---\n");
    bool result3 = change_email(testUsername, updatedEmail, "another.email@example.com", "WrongPassword!");
    printf("Result: %s\n", result3 ? "SUCCESS" : "FAILURE");
    printf("Current email: %s\n\n", testUser->email);

    // Test Case 4: Failure - Incorrect old email
    printf("--- Test Case 4: Failure (Incorrect Old Email) ---\n");
    bool result4 = change_email(testUsername, "wrong.old.email@example.com", "another.email@example.com", userPassword);
    printf("Result: %s\n", result4 ? "SUCCESS" : "FAILURE");
    printf("Current email: %s\n\n", testUser->email);

    // Test Case 5: Failure - New email is same as old
    printf("--- Test Case 5: Failure (New Email is Same as Old) ---\n");
    bool result5 = change_email(testUsername, updatedEmail, updatedEmail, userPassword);
    printf("Result: %s\n", result5 ? "SUCCESS" : "FAILURE");
    printf("Current email: %s\n\n", testUser->email);

    return 0;
}