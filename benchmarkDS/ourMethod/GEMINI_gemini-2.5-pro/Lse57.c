#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Fixed maximums for this simple example
#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_EMAIL_LEN 100
#define MAX_PASSWORD_LEN 100 // Should not be stored, just for input buffers
#define SALT_LEN 16
#define HASH_LEN 64 // Placeholder length

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    char email[MAX_EMAIL_LEN + 1];
    char passwordHash[HASH_LEN + 1];
    char salt[SALT_LEN + 1];
} User;

// In-memory "database" and session state
static User user_database[MAX_USERS];
static int user_count = 0;
static User* loggedInUser = NULL;

// WARNING: This is a placeholder for a real cryptographic salt generator.
// In a production environment, use a CSPRNG from a library like OpenSSL or the OS's API.
void generate_salt(char* salt_buffer) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    srand((unsigned int)time(NULL)); // Note: time() is not cryptographically secure. For demonstration only.
    for (int i = 0; i < SALT_LEN; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        salt_buffer[i] = charset[key];
    }
    salt_buffer[SALT_LEN] = '\0';
}

// WARNING: This is NOT a secure password hash. It's a simple placeholder.
// In a production system, you MUST use a standard, strong key derivation function
// like Argon2, scrypt, or PBKDF2 from a vetted cryptography library (e.g., OpenSSL).
void hash_password(const char* password, const char* salt, char* hash_buffer) {
    // This is a trivial "hash" for demonstration only. DO NOT USE IN PRODUCTION.
    char to_hash[MAX_PASSWORD_LEN + SALT_LEN + 1];
    snprintf(to_hash, sizeof(to_hash), "%s%s", password, salt);
    
    // Using a simple algorithm for demonstration.
    unsigned long hash = 5381;
    int c;
    char* str = to_hash;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    snprintf(hash_buffer, HASH_LEN + 1, "%lu", hash);
}

bool verify_password(const char* password, const char* originalHash, const char* salt) {
    char comparisonHash[HASH_LEN + 1];
    hash_password(password, salt, comparisonHash);
    return strcmp(originalHash, comparisonHash) == 0;
}

// Basic email validation
bool is_valid_email(const char* email) {
    if (email == NULL) return false;
    const char *at = strchr(email, '@');
    if (!at) return false;
    const char *dot = strrchr(at, '.');
    return dot && dot > at + 1 && dot[1] != '\0';
}

bool registerUser(const char* username, const char* password, const char* email) {
    // Rule #1: Input Validation
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0 || !is_valid_email(email)) {
        printf("Registration failed: Invalid input.\n");
        return false;
    }
    if (user_count >= MAX_USERS) {
        printf("Registration failed: Database full.\n");
        return false;
    }
    if (strlen(username) > MAX_USERNAME_LEN || strlen(email) > MAX_EMAIL_LEN) {
        printf("Registration failed: Input too long.\n");
        return false;
    }

    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_database[i].username, username) == 0) {
            printf("Registration failed: Username already exists.\n");
            return false;
        }
    }

    User* newUser = &user_database[user_count];
    strncpy(newUser->username, username, MAX_USERNAME_LEN);
    newUser->username[MAX_USERNAME_LEN] = '\0';

    strncpy(newUser->email, email, MAX_EMAIL_LEN);
    newUser->email[MAX_EMAIL_LEN] = '\0';

    generate_salt(newUser->salt);
    hash_password(password, newUser->salt, newUser->passwordHash);
    
    user_count++;
    printf("User %s registered successfully.\n", username);
    return true;
}

bool login(const char* username, const char* password) {
    if (!username || !password || strlen(username) == 0 || strlen(password) == 0) {
        return false;
    }
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_database[i].username, username) == 0) {
            if (verify_password(password, user_database[i].passwordHash, user_database[i].salt)) {
                loggedInUser = &user_database[i];
                printf("%s logged in successfully.\n", username);
                return true;
            }
        }
    }
    printf("Login failed for %s.\n", username);
    return false;
}

void logout() {
    if (loggedInUser) {
        printf("%s logged out.\n", loggedInUser->username);
        loggedInUser = NULL;
    }
}

bool changeEmail(const char* oldEmail, const char* newEmail, const char* confirmPassword) {
    // Rule #1: Input Validation
    if (!oldEmail || !confirmPassword || strlen(oldEmail) == 0 || !is_valid_email(newEmail) || strlen(confirmPassword) == 0) {
        printf("Email change failed: Invalid input provided.\n");
        return false;
    }
    if (strlen(newEmail) > MAX_EMAIL_LEN) {
        printf("Email change failed: New email is too long.\n");
        return false;
    }

    if (!loggedInUser) {
        printf("Email change failed: User must be logged in.\n");
        return false;
    }

    if (strcmp(loggedInUser->email, oldEmail) != 0) {
        printf("Email change failed: Old email does not match.\n");
        return false;
    }

    if (!verify_password(confirmPassword, loggedInUser->passwordHash, loggedInUser->salt)) {
        printf("Email change failed: Incorrect password.\n");
        return false;
    }

    strncpy(loggedInUser->email, newEmail, MAX_EMAIL_LEN);
    loggedInUser->email[MAX_EMAIL_LEN] = '\0';
    printf("Email for user %s changed successfully to %s\n", loggedInUser->username, newEmail);
    return true;
}

void printUserDetails(const char* username) {
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_database[i].username, username) == 0) {
            printf("Details for %s: Email = %s\n", username, user_database[i].email);
            return;
        }
    }
    printf("User %s not found.\n", username);
}

int main() {
    // Setup: Register a user
    registerUser("c_user", "CPass123#", "initial_c@example.com");
    printf("----------------------------------------\n");
    
    // Test Case 1: Successful email change
    printf("Test Case 1: Successful email change\n");
    login("c_user", "CPass123#");
    printUserDetails("c_user");
    changeEmail("initial_c@example.com", "updated_c@example.com", "CPass123#");
    printUserDetails("c_user");
    logout();
    printf("----------------------------------------\n");

    // Test Case 2: Attempt to change email when not logged in
    printf("Test Case 2: Attempt change when not logged in\n");
    changeEmail("updated_c@example.com", "another_c@example.com", "CPass123#");
    printUserDetails("c_user");
    printf("----------------------------------------\n");

    // Test Case 3: Attempt to change email with wrong password
    printf("Test Case 3: Attempt change with wrong password\n");
    login("c_user", "CPass123#");
    changeEmail("updated_c@example.com", "another_c@example.com", "WrongPassword!");
    printUserDetails("c_user");
    logout();
    printf("----------------------------------------\n");

    // Test Case 4: Attempt to change email with wrong old email
    printf("Test Case 4: Attempt change with wrong old email\n");
    login("c_user", "CPass123#");
    changeEmail("wrong.old_c@example.com", "another_c@example.com", "CPass123#");
    printUserDetails("c_user");
    logout();
    printf("----------------------------------------\n");

    // Test Case 5: Attempt to change to an invalid new email
    printf("Test Case 5: Attempt change to an invalid new email\n");
    login("c_user", "CPass123#");
    changeEmail("updated_c@example.com", "invalid-email", "CPass123#");
    printUserDetails("c_user");
    logout();
    printf("----------------------------------------\n");
    
    return 0;
}