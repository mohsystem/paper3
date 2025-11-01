#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_HASH_LEN 100

// Struct to represent a user record
typedef struct {
    char username[MAX_USERNAME_LEN];
    char stored_password[MAX_HASH_LEN];
} User;

// In-memory array to simulate a database.
// A real application would use a proper database system.
static User user_database[MAX_USERS];
static int user_count = 0;


/**
 * SECURE HASHING PLACEHOLDER
 *
 * WARNING: THIS IS NOT A SECURE HASHING FUNCTION.
 * The C standard library does not provide cryptographic functions. For a
 * real-world, secure application, you MUST use a well-vetted, external
 * library like libsodium or OpenSSL.
 *
 * This function is a placeholder to demonstrate the application's logic flow.
 * It returns a newly allocated string that must be freed by the caller.
 * It provides NO actual security.
 */
char* insecure_placeholder_hash(const char* password) {
    // This is a dummy salt. A real implementation would generate a unique,
    // cryptographically random salt for each user.
    const char* salt = "dummy_salt:";
    size_t len = strlen(salt) + strlen(password) + 1;
    char* hashed_password = (char*)malloc(len);
    if (hashed_password == NULL) {
        perror("Failed to allocate memory for hash");
        return NULL;
    }
    // Insecure: simply concatenates salt and password
    strcpy(hashed_password, salt);
    strcat(hashed_password, password);
    return hashed_password;
}


/**
 * Registers a new user by "hashing" their password and storing it.
 * @param username The username for the new account.
 * @param password The plain-text password for the new account.
 * @return 1 on success, 0 on failure.
 */
int registerUser(const char* username, const char* password) {
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        printf("Registration failed: Username and password cannot be empty.\n");
        return 0;
    }

    if (user_count >= MAX_USERS) {
        printf("Registration failed: Database is full.\n");
        return 0;
    }
    
    if (strlen(username) >= MAX_USERNAME_LEN) {
        printf("Registration failed: Username is too long.\n");
        return 0;
    }

    // Check for duplicate username
    for (int i = 0; i < user_count; i++) {
        if (strcmp(user_database[i].username, username) == 0) {
            printf("Registration failed: Username '%s' already exists.\n", username);
            return 0;
        }
    }

    char* hashed_password = insecure_placeholder_hash(password);
    if (hashed_password == NULL) {
        printf("Registration failed: Could not hash password.\n");
        return 0;
    }
    
    if (strlen(hashed_password) >= MAX_HASH_LEN) {
        printf("Registration failed: Generated hash is too long.\n");
        free(hashed_password);
        return 0;
    }

    // Add user to the database
    strcpy(user_database[user_count].username, username);
    strcpy(user_database[user_count].stored_password, hashed_password);
    user_count++;
    
    printf("User '%s' registered successfully.\n", username);
    free(hashed_password); // Free the memory allocated by the hash function
    return 1;
}

int main() {
    printf("--- Running 5 Test Cases for User Registration ---\n");
    printf("WARNING: Using an insecure placeholder for password hashing.\n");

    // Test Case 1: Successful registration
    registerUser("alice", "Password123!");

    // Test Case 2: Successful registration with a different user
    registerUser("bob", "SecurePass@2023");

    // Test Case 3: Attempt to register a duplicate username
    registerUser("alice", "AnotherPassword");

    // Test Case 4: Registration with an empty username
    registerUser("", "some_password");
    
    // Test Case 5: Successful registration
    registerUser("david", "MyP@ssw0rd");

    printf("\n--- Final Simulated Database State ---\n");
    // Note: In a real application, NEVER print sensitive data like this.
    // This is for demonstration purposes only.
    for (int i = 0; i < user_count; i++) {
        printf("Username: %s, Stored Value: %s\n", user_database[i].username, user_database[i].stored_password);
    }

    return 0;
}