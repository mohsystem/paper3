#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

// ============================= SECURITY WARNING =================================
// The C standard library does NOT include cryptographic functions.
// The hashing function (`pseudo_hash`) below is for DEMONSTRATION PURPOSES ONLY.
// It illustrates the concepts of salting and iteration but is NOT
// CRYPTOGRAPHICALLY SECURE and MUST NOT be used in a real application.
//
// In a real-world C application, you MUST use a well-vetted, reputable
// cryptography library like OpenSSL, libsodium, or libgcrypt to handle password
// hashing (e.g., with Argon2, scrypt, or PBKDF2).
// ==============================================================================

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 50
#define HASH_LEN 64
#define SALT_LEN 64
#define MAX_USERS 10

typedef struct {
    char username[MAX_USERNAME_LEN];
    char salt[SALT_LEN];
    char hashedPassword[HASH_LEN];
} UserData;

// In-memory user store to simulate a database.
UserData userDatabase[MAX_USERS];
int userCount = 0;

/**
 * A NON-SECURE pseudo-hashing function for demonstration purposes.
 * DO NOT USE THIS IN PRODUCTION.
 */
void pseudo_hash(const char* password, const char* salt, char* output) {
    char combined[MAX_PASSWORD_LEN + SALT_LEN];
    snprintf(combined, sizeof(combined), "%s%s", password, salt);

    unsigned long long hash = 5381; // A simple starting prime for djb2
    for (int i = 0; i < 10000; ++i) { // Fake "iterations"
        for (int j = 0; combined[j] != '\0'; ++j) {
            hash = ((hash << 5) + hash) + combined[j]; // hash * 33 + c
        }
        snprintf(combined, sizeof(combined), "%llu", hash);
    }
    snprintf(output, HASH_LEN, "%llu", hash);
}

/**
 * Compares two strings in a way that takes a constant amount of time
 * to help prevent timing attacks.
 */
bool secure_compare(const char* s1, const char* s2, size_t len) {
    // Note: A simple `strcmp` would short-circuit and be vulnerable.
    int result = 0;
    for (size_t i = 0; i < len; ++i) {
        result |= s1[i] ^ s2[i];
    }
    return result == 0;
}


void to_lowercase(const char* input, char* output, size_t max_len) {
    for (size_t i = 0; i < max_len - 1 && input[i] != '\0'; ++i) {
        output[i] = tolower((unsigned char)input[i]);
        output[i + 1] = '\0';
    }
}

/**
 * Simulates user registration.
 */
void registerUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS) {
        printf("Database is full. Cannot register new user.\n");
        return;
    }
    if (strlen(username) == 0 || strlen(password) == 0) {
        printf("Registration failed: Username and password cannot be empty.\n");
        return;
    }

    strncpy(userDatabase[userCount].username, username, MAX_USERNAME_LEN - 1);
    userDatabase[userCount].username[MAX_USERNAME_LEN - 1] = '\0';

    snprintf(userDatabase[userCount].salt, SALT_LEN, "salt_for_%s", username);
    
    pseudo_hash(password, userDatabase[userCount].salt, userDatabase[userCount].hashedPassword);
    
    userCount++;
}

/**
 * Simulates a login attempt. The function returns a dynamically allocated string,
 * which the caller is responsible for freeing.
 * For this example, we will just print the result and not manage memory.
 */
void login(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        printf("Login failed: Username or password cannot be empty. Redirecting to login page.\n");
        return;
    }

    char lower_username[MAX_USERNAME_LEN];
    to_lowercase(username, lower_username, MAX_USERNAME_LEN);
    
    UserData* user = NULL;
    // Find user (case-insensitive search)
    for (int i = 0; i < userCount; ++i) {
        char db_lower_username[MAX_USERNAME_LEN];
        to_lowercase(userDatabase[i].username, db_lower_username, MAX_USERNAME_LEN);
        if (strcmp(lower_username, db_lower_username) == 0) {
            user = &userDatabase[i];
            break;
        }
    }

    if (user != NULL) {
        char providedHash[HASH_LEN];
        pseudo_hash(password, user->salt, providedHash);

        if (secure_compare(providedHash, user->hashedPassword, HASH_LEN)) {
            printf("Login successful for %s! Redirecting to dashboard.\n", user->username);
            return;
        }
    }

    printf("Login failed: Invalid username or password. Redirecting to login page.\n");
}

int main() {
    // --- Setup: Simulate user registration ---
    registerUser("admin", "AdminPassword123!");
    registerUser("jdoe", "P@ssw0rd_jdoe");
    printf("--- User database initialized ---\n");

    // --- Test Cases ---
    printf("\n--- Running Test Cases ---\n");

    // Test Case 1: Correct username and password
    printf("1. ");
    login("admin", "AdminPassword123!");

    // Test Case 2: Correct username, incorrect password
    printf("2. ");
    login("admin", "WrongPassword");

    // Test Case 3: Incorrect username
    printf("3. ");
    login("unknownUser", "anyPassword");

    // Test Case 4: Correct username (different case), correct password
    printf("4. ");
    login("Admin", "AdminPassword123!");

    // Test Case 5: Empty password
    printf("5. ");
    login("jdoe", "");

    // Test Case 6: Empty username
    printf("6. ");
    login("", "P@ssw0rd_jdoe");
    
    // Test Case 7: Null password
    printf("7. ");
    login("jdoe", NULL);

    return 0;
}