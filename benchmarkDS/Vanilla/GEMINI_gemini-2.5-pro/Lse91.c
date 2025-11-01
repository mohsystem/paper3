#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 10
#define HASH_BUFFER_SIZE 20 // Sufficient for hex representation of unsigned long

typedef struct {
    char* username;
    char* password_hash;
} User;

// In-memory "database". In a real application, this would be a persistent database.
User userDatabase[MAX_USERS];
int userCount = 0;

/**
 * @brief Hashes a password using a simple, non-cryptographic algorithm.
 * @note In a real-world application, use a well-vetted cryptographic library
 *       like OpenSSL, libsodium, or bcrypt for password hashing. This simple
 *       hash is for demonstration purposes only.
 * @param password The plain-text password to hash.
 * @return A dynamically allocated string containing the hash. The caller must free this memory.
 */
char* hashPassword(const char* password) {
    // Using DJB2 hash algorithm for simplicity
    unsigned long hash = 5381;
    int c;
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    char* hash_str = (char*)malloc(HASH_BUFFER_SIZE);
    if (hash_str == NULL) {
        fprintf(stderr, "Memory allocation failed in hashPassword\n");
        exit(EXIT_FAILURE);
    }
    snprintf(hash_str, HASH_BUFFER_SIZE, "%lx", hash);
    return hash_str;
}

/**
 * @brief Registers a new user by hashing their password and storing it.
 * @param username The username for the new account.
 * @param password The plain-text password for the new account.
 * @return 1 if registration is successful, 0 otherwise.
 */
int registerUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS) {
        printf("Registration failed: Database is full.\n");
        return 0;
    }

    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            printf("Registration failed: Username '%s' already exists.\n", username);
            return 0;
        }
    }

    char* hashedPassword = hashPassword(password);

    userDatabase[userCount].username = (char*)malloc(strlen(username) + 1);
    userDatabase[userCount].password_hash = (char*)malloc(strlen(hashedPassword) + 1);

    if (userDatabase[userCount].username == NULL || userDatabase[userCount].password_hash == NULL) {
        fprintf(stderr, "Memory allocation failed for user data\n");
        free(hashedPassword);
        free(userDatabase[userCount].username);
        free(userDatabase[userCount].password_hash);
        exit(EXIT_FAILURE);
    }
    
    strcpy(userDatabase[userCount].username, username);
    strcpy(userDatabase[userCount].password_hash, hashedPassword);
    
    free(hashedPassword); // The data is now copied, so we can free the temp hash string

    printf("User '%s' registered successfully.\n", username);
    userCount++;
    return 1;
}

/**
 * @brief Frees all dynamically allocated memory used by the database.
 */
void cleanupDatabase() {
    for (int i = 0; i < userCount; ++i) {
        free(userDatabase[i].username);
        free(userDatabase[i].password_hash);
    }
}

int main() {
    printf("--- C User Registration System ---\n");
    
    // Test Case 1: Successful registration
    registerUser("alice", "password123");
    
    // Test Case 2: Another successful registration
    registerUser("bob", "bob_secure_pass");
    
    // Test Case 3: A third successful registration
    registerUser("charlie", "qwerty");
    
    // Test Case 4: Attempt to register a duplicate username
    registerUser("alice", "another_password");
    
    // Test Case 5: A final successful registration
    registerUser("david", "d@v1d_pa$$");
    
    printf("\n--- Current Database ---\n");
    for (int i = 0; i < userCount; ++i) {
        printf("Username: %s, Hashed Password: %s\n", userDatabase[i].username, userDatabase[i].password_hash);
    }

    // Free all allocated memory before exiting
    cleanupDatabase();

    return 0;
}