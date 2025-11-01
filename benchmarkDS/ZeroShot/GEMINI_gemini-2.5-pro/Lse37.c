#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// --- SECURITY WARNING ---
// C does not have a standard cryptography library. For a real-world, secure application,
// you MUST use a well-vetted, external library like OpenSSL (libcrypto) or libsodium.
// The hash function below is an INSECURE PLACEHOLDER to demonstrate the logic of
// salting and constant-time comparison. DO NOT USE THIS HASH FUNCTION IN PRODUCTION.
// A real application should use an adaptive hashing function like Argon2 or BCrypt.

#define MAX_USERNAME_LEN 50
#define MAX_SALT_LEN 32
#define MAX_HASH_LEN 65 // SHA-256 hex string + null terminator
#define DB_SIZE 10

// --- INSECURE PLACEHOLDER HASH FUNCTION ---
// This function simulates hashing. Replace with a real crypto library call.
void insecure_placeholder_hash(const char* password, const char* salt, char* output_hash) {
    char to_hash[256];
    snprintf(to_hash, sizeof(to_hash), "%s%s", salt, password);
    
    unsigned long hash = 5381;
    int c;
    char* ptr = to_hash;
    while ((c = *ptr++)) {
        hash = ((hash << 5) + hash) + c; // djb2 algorithm
    }
    snprintf(output_hash, MAX_HASH_LEN, "%lu", hash);
}

typedef struct {
    char username[MAX_USERNAME_LEN];
    char salt[MAX_SALT_LEN];
    char hashedPassword[MAX_HASH_LEN];
} UserCredentials;

// Simulates a database. A real app would use a database.
static UserCredentials userDatabase[DB_SIZE];
static int userCount = 0;

/**
 * Performs a constant-time comparison of two strings.
 * Returns 0 if equal, non-zero otherwise.
 */
int constant_time_compare(const char* a, const char* b) {
    size_t len_a = strlen(a);
    if (len_a != strlen(b)) {
        return 1; // Not equal
    }

    int result = 0;
    for (size_t i = 0; i < len_a; ++i) {
        result |= a[i] ^ b[i];
    }
    return result;
}

const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL || username[0] == '\0' || password[0] == '\0') {
        return "cant_login";
    }

    UserCredentials* user = NULL;
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            user = &userDatabase[i];
            break;
        }
    }

    char attemptedHash[MAX_HASH_LEN];
    if (user == NULL) {
        // To prevent timing attacks, perform a dummy hash and comparison.
        const char* dummy_salt = "dummy_salt_value_for_non_existent_user";
        insecure_placeholder_hash(password, dummy_salt, attemptedHash);
        constant_time_compare(attemptedHash, "a_dummy_hash_that_will_fail");
        return "cant_login";
    }

    insecure_placeholder_hash(password, user->salt, attemptedHash);

    if (constant_time_compare(attemptedHash, user->hashedPassword) == 0) {
        return "success";
    } else {
        return "cant_login";
    }
    
    // In a more complex function, you might have error paths.
    // This simple example doesn't have a natural path to an internal server error,
    // but in a real app, memory allocation failures, file I/O errors, etc., would lead here.
    // return "internal_server_error";
}

void setupTestDatabase() {
    if (userCount >= DB_SIZE) {
        printf("Database is full.\n");
        return;
    }
    const char* testUser = "alice";
    const char* testPass = "password123";
    // In a real app, use a cryptographically secure random number generator for the salt.
    const char* salt = "random_salt_for_alice_c_123";

    strncpy(userDatabase[userCount].username, testUser, MAX_USERNAME_LEN - 1);
    userDatabase[userCount].username[MAX_USERNAME_LEN - 1] = '\0';

    strncpy(userDatabase[userCount].salt, salt, MAX_SALT_LEN - 1);
    userDatabase[userCount].salt[MAX_SALT_LEN - 1] = '\0';
    
    insecure_placeholder_hash(testPass, salt, userDatabase[userCount].hashedPassword);
    
    printf("Test database setup for user: %s\n", userDatabase[userCount].username);
    printf("Salt: %s\n", userDatabase[userCount].salt);
    printf("Hashed Password: %s\n", userDatabase[userCount].hashedPassword);
    printf("------------------------------------\n");
    
    userCount++;
}

int main() {
    setupTestDatabase();

    // Test Cases
    printf("Test Case 1: Correct credentials\n");
    printf("Login for 'alice' with 'password123': %s\n\n", login("alice", "password123"));

    printf("Test Case 2: Correct username, incorrect password\n");
    printf("Login for 'alice' with 'wrongpassword': %s\n\n", login("alice", "wrongpassword"));
    
    printf("Test Case 3: Incorrect username\n");
    printf("Login for 'bob' with 'password123': %s\n\n", login("bob", "password123"));

    printf("Test Case 4: Empty password\n");
    printf("Login for 'alice' with '': %s\n\n", login("alice", ""));

    printf("Test Case 5: NULL username\n");
    printf("Login for NULL with 'password123': %s\n\n", login(NULL, "password123"));

    return 0;
}