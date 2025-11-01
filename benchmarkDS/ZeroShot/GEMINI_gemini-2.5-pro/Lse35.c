#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// --- SECURITY WARNING ---
// This C code is for demonstration purposes ONLY. Standard C lacks a built-in,
// secure cryptographic library for password hashing. The "hashing" used here is
// a simple string manipulation and is COMPLETELY INSECURE.
//
// In a real-world application, you MUST use a proper, vetted cryptographic library
// like OpenSSL or libsodium to implement a strong key derivation
// function such as Argon2, scrypt, or bcrypt. NEVER implement your own crypto
// and never store passwords in plaintext or with weak/fake hashing like this.

#define MAX_USERS 10
#define SALT_LENGTH 16
#define MAX_USERNAME_LEN 50
#define MAX_PASS_LEN 50
#define HASH_BUFFER_SIZE 256

// Mock database using a simple struct array
typedef struct {
    char* username;
    char* saltAndHash; // Stored as "salt:hash"
} User;

static User userDatabase[MAX_USERS];
static int userCount = 0;

/**
 * "Hashes" a password using an INSECURE method for demonstration.
 * WARNING: This is NOT a real hash function. Do not use in production.
 * The caller is responsible for freeing the returned memory.
 */
char* hash_password(const char* password, const char* salt) {
    char* hash_buffer = (char*)malloc(HASH_BUFFER_SIZE);
    if (hash_buffer == NULL) {
        perror("Failed to allocate memory for hash");
        return NULL;
    }
    snprintf(hash_buffer, HASH_BUFFER_SIZE, "hashed(%s%s)", password, salt);
    return hash_buffer;
}

/**
 * Generates a pseudo-random salt.
 * WARNING: This is NOT cryptographically secure. For demonstration only.
 */
void generate_salt(char salt_buffer[SALT_LENGTH + 1]) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < SALT_LENGTH; i++) {
        salt_buffer[i] = charset[rand() % (sizeof(charset) - 1)];
    }
    salt_buffer[SALT_LENGTH] = '\0';
}

/**
 * Registers a new user by "hashing" their password and storing it.
 */
void registerUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS || username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return;
    }
    
    char salt[SALT_LENGTH + 1];
    generate_salt(salt);
    
    char* hashedPassword = hash_password(password, salt);
    if (hashedPassword == NULL) return;

    // Allocate memory for username and salt:hash string
    userDatabase[userCount].username = strdup(username);
    
    size_t saltAndHashLen = strlen(salt) + 1 + strlen(hashedPassword) + 1;
    userDatabase[userCount].saltAndHash = (char*)malloc(saltAndHashLen);
    
    if (userDatabase[userCount].username == NULL || userDatabase[userCount].saltAndHash == NULL) {
         perror("Failed to allocate memory for user registration");
         free(hashedPassword);
         free(userDatabase[userCount].username); // In case one succeeded and the other failed
         return;
    }

    snprintf(userDatabase[userCount].saltAndHash, saltAndHashLen, "%s:%s", salt, hashedPassword);
    
    free(hashedPassword);
    userCount++;
}

/**
 * Simulates a login attempt.
 */
bool login(const char* username, const char* password) {
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        return false;
    }

    User* foundUser = NULL;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            foundUser = &userDatabase[i];
            break;
        }
    }

    if (foundUser == NULL) {
        // User not found. Perform dummy hash to mitigate timing attacks.
        char dummy_salt[SALT_LENGTH + 1];
        generate_salt(dummy_salt);
        char* dummy_hash = hash_password("dummy_password", dummy_salt);
        free(dummy_hash);
        return false;
    }

    // Extract salt and hash from the stored string
    char* storedData = strdup(foundUser->saltAndHash);
    if(storedData == NULL) return false;

    char* salt = strtok(storedData, ":");
    char* storedHash = strtok(NULL, ":");

    if (salt == NULL || storedHash == NULL) {
        free(storedData);
        return false; // Malformed data
    }
    
    char* providedPasswordHash = hash_password(password, salt);
    if (providedPasswordHash == NULL) {
        free(storedData);
        return false;
    }

    // In a real application, this comparison MUST be constant-time.
    bool success = (strcmp(storedHash, providedPasswordHash) == 0);

    free(providedPasswordHash);
    free(storedData);
    
    return success;
}

void cleanup() {
    for (int i = 0; i < userCount; i++) {
        free(userDatabase[i].username);
        free(userDatabase[i].saltAndHash);
    }
}

int main() {
    // Seed the random number generator (for insecure salt generation)
    srand((unsigned int)time(NULL));

    // Setup: Register some users in our mock database
    registerUser("admin", "S3cureP@ssword123!");
    registerUser("user1", "myWeakPassword");

    printf("--- Running 5 Test Cases ---\n");

    // Test Case 1: Successful login for admin
    bool test1 = login("admin", "S3cureP@ssword123!");
    printf("Test 1 (Correct admin password): %s\n", test1 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page");

    // Test Case 2: Failed login for admin (wrong password)
    bool test2 = login("admin", "wrongpassword");
    printf("Test 2 (Incorrect admin password): %s\n", test2 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page");

    // Test Case 3: Failed login for non-existent user
    bool test3 = login("nonexistentuser", "somepassword");
    printf("Test 3 (Non-existent user): %s\n", test3 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page");

    // Test Case 4: Successful login for another user
    bool test4 = login("user1", "myWeakPassword");
    printf("Test 4 (Correct user1 password): %s\n", test4 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page");

    // Test Case 5: Failed login with empty credentials
    bool test5 = login("", "S3cureP@ssword123!");
    printf("Test 5 (Empty username): %s\n", test5 ? "SUCCESS -> Redirect to home page" : "FAIL -> Redirect to login page");
    
    cleanup(); // Free allocated memory
    return 0;
}