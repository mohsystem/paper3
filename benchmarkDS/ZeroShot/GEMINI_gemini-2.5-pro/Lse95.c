/*
 * IMPORTANT: This code requires the OpenSSL library.
 * To compile, you need to link against libssl and libcrypto.
 * Example compilation command:
 * gcc your_file_name.c -o your_executable -lssl -lcrypto
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <openssl/evp.h>

#define SALT_LENGTH 16
#define ITERATION_COUNT 65536
#define KEY_LENGTH 32 // For SHA256
#define MAX_USERNAME_LEN 50
#define MAX_USERS 10

// Struct to hold user credentials
typedef struct {
    char username[MAX_USERNAME_LEN];
    unsigned char salt[SALT_LENGTH];
    unsigned char hashedPassword[KEY_LENGTH];
} UserCredentials;

// In-memory array to simulate a database
static UserCredentials userDatabase[MAX_USERS];
static int userCount = 0;

// Helper to print hex data
void print_hex(const char* label, const unsigned char* data, int len) {
    printf("%s", label);
    for (int i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

/**
 * Finds a user in the database by username.
 * @param username The username to find.
 * @return A pointer to the UserCredentials struct, or NULL if not found.
 */
UserCredentials* findUser(const char* username) {
    for (int i = 0; i < userCount; ++i) {
        if (strncmp(userDatabase[i].username, username, MAX_USERNAME_LEN) == 0) {
            return &userDatabase[i];
        }
    }
    return NULL;
}

/**
 * Hashes the password using PBKDF2.
 * @param password The password string.
 * @param salt The salt buffer.
 * @param hash_out The output buffer for the hash.
 * @return 1 on success, 0 on failure.
 */
int hashPassword(const char* password, const unsigned char* salt, unsigned char* hash_out) {
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_LENGTH,
        ITERATION_COUNT,
        EVP_sha256(),
        KEY_LENGTH,
        hash_out
    );
    return result == 1;
}

/**
 * Registers a new user.
 * @param username The username.
 * @param password The plaintext password.
 * @return 1 on success, 0 on failure (user exists, db full, etc.).
 */
int registerUser(const char* username, const char* password) {
    if (username == NULL || strlen(username) == 0 || password == NULL || strlen(password) == 0) {
        printf("Username and password cannot be empty.\n");
        return 0;
    }
    if (userCount >= MAX_USERS) {
        return 0; // Database is full
    }
    if (findUser(username) != NULL) {
        return 0; // User already exists
    }

    UserCredentials* newUser = &userDatabase[userCount];
    
    // Copy username safely
    strncpy(newUser->username, username, MAX_USERNAME_LEN - 1);
    newUser->username[MAX_USERNAME_LEN - 1] = '\0';

    // Generate salt
    if (RAND_bytes(newUser->salt, SALT_LENGTH) != 1) {
        return 0; // Salt generation failed
    }

    // Hash password
    if (!hashPassword(password, newUser->salt, newUser->hashedPassword)) {
        return 0; // Hashing failed
    }

    userCount++;
    return 1;
}

/**
 * Verifies a user's password.
 * @param username The username.
 * @param password The plaintext password to check.
 * @return 1 if password is correct, 0 otherwise.
 */
int verifyPassword(const char* username, const char* password) {
    UserCredentials* user = findUser(username);
    if (user == NULL) {
        return 0; // User not found
    }

    unsigned char providedPasswordHash[KEY_LENGTH];
    if (!hashPassword(password, user->salt, providedPasswordHash)) {
        return 0; // Hashing failed
    }

    // Constant-time comparison
    return CRYPTO_memcmp(providedPasswordHash, user->hashedPassword, KEY_LENGTH) == 0;
}


int main() {
    printf("--- C Test Cases ---\n");
    
    // Test Case 1: Successful registration
    int reg1 = registerUser("alice", "Password123!");
    printf("Test 1 - Alice registration: %s\n", reg1 ? "Success" : "Fail");

    // Test Case 2: Successful login
    int login1 = verifyPassword("alice", "Password123!");
    printf("Test 2 - Alice successful login: %s\n", login1 ? "Success" : "Fail");

    // Test Case 3: Failed login (wrong password)
    int login2 = verifyPassword("alice", "WrongPassword");
    printf("Test 3 - Alice failed login (wrong password): %s\n", !login2 ? "Success" : "Fail");

    // Test Case 4: Failed login (non-existent user)
    int login3 = verifyPassword("bob", "Password123!");
    printf("Test 4 - Bob failed login (user not found): %s\n", !login3 ? "Success" : "Fail");

    // Test Case 5: Failed registration (duplicate username)
    int reg2 = registerUser("alice", "AnotherPassword");
    printf("Test 5 - Alice duplicate registration: %s\n", !reg2 ? "Success" : "Fail");

    // Optional: Print stored data for verification
    UserCredentials* alice = findUser("alice");
    if (alice != NULL) {
        printf("\nStored data for Alice:\n");
        print_hex("Salt: ", alice->salt, SALT_LENGTH);
        print_hex("Hashed Password: ", alice->hashedPassword, KEY_LENGTH);
    }

    return 0;
}