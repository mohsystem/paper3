#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// !!!!!!!!!!!!!!!!!!!!!!!!!!!  SECURITY WARNING  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// This C code is for DEMONSTRATION PURPOSES ONLY. It illustrates the
// architectural pattern of salting and hashing but does NOT use
// cryptographically secure functions, as they are not in the C standard library.
//
// IN A REAL-WORLD APPLICATION, YOU MUST USE A DEDICATED CRYPTOGRAPHY LIBRARY
// like OpenSSL or libsodium to handle:
//
// 1.  SECURE HASHING: Use a strong key derivation function like Argon2, bcrypt,
//     or PBKDF2. The simple string concatenation used here is COMPLETELY INSECURE.
//
// 2.  CONSTANT-TIME COMPARISON: Use a function like `CRYPTO_memcmp` from
//     OpenSSL to prevent timing attacks. The standard `strcmp` is not secure for this.
//
// 3.  CRYPTOGRAPHICALLY SECURE RANDOMNESS: Use the library's secure random
//     number generator for salts. `rand()` is predictable and must not be used.
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#define MAX_USERS 10
#define MAX_STRING_LEN 256
#define SALT_LEN 16

typedef struct {
    char username[MAX_STRING_LEN];
    char salt[SALT_LEN + 1];
    char hashedPassword[MAX_STRING_LEN + SALT_LEN]; 
} UserCredentials;

// Simulated in-memory database
UserCredentials userDatabase[MAX_USERS];
int userCount = 0;

// **INSECURE** placeholder for generating a salt. Use a real crypto library.
void generateSalt(char* salt_buffer) {
    const char charset[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    for (int i = 0; i < SALT_LEN; i++) {
        int key = rand() % (int)(sizeof(charset) - 1);
        salt_buffer[i] = charset[key];
    }
    salt_buffer[SALT_LEN] = '\0';
}

// **INSECURE** placeholder for a hashing function. Use a real crypto library.
void insecureHashFunction(const char* password, const char* salt, char* hash_buffer) {
    // This is NOT a hash function. It's for demonstrating the data flow ONLY.
    snprintf(hash_buffer, MAX_STRING_LEN + SALT_LEN, "hash_of(%s%s)", salt, password);
}

void registerUser(const char* username, const char* password) {
    if (userCount >= MAX_USERS) {
        printf("Registration failed: Database is full.\n");
        return;
    }
    if (username == NULL || password == NULL || strlen(username) == 0 || strlen(password) == 0) {
        printf("Registration failed: Username and password cannot be empty.\n");
        return;
    }

    strncpy(userDatabase[userCount].username, username, MAX_STRING_LEN - 1);
    userDatabase[userCount].username[MAX_STRING_LEN - 1] = '\0';

    generateSalt(userDatabase[userCount].salt);
    insecureHashFunction(password, userDatabase[userCount].salt, userDatabase[userCount].hashedPassword);
    
    userCount++;
    printf("User '%s' registered successfully (using insecure demo methods).\n", username);
}

// Returns a dynamically allocated string with the result. Caller MUST free it.
char* loginUser(const char* username, const char* password) {
    const char* failureMsg = "Login failed: Invalid username or password. Redirecting to login page.";
    const char* successMsg = "Login successful! Redirecting to dashboard...";
    char* result = (char*)malloc(sizeof(char) * 128);

    if (result == NULL) {
        perror("Failed to allocate memory for result string");
        exit(EXIT_FAILURE);
    }

    UserCredentials* user = NULL;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            user = &userDatabase[i];
            break;
        }
    }

    // Generic message to prevent username enumeration.
    if (user == NULL || password == NULL || strlen(password) == 0) {
        strcpy(result, failureMsg);
        return result;
    }

    char providedPasswordHash[MAX_STRING_LEN + SALT_LEN];
    insecureHashFunction(password, user->salt, providedPasswordHash);

    // **INSECURE COMPARISON**: `strcmp` is vulnerable to timing attacks. Use a constant-time comparison function.
    if (strcmp(user->hashedPassword, providedPasswordHash) == 0) {
        strcpy(result, successMsg);
    } else {
        strcpy(result, failureMsg);
    }
    
    return result;
}

int main() {
    // Seed the insecure random number generator. A real app needs a better source of entropy.
    srand((unsigned int)time(NULL));

    // Setup: Register a user for testing
    const char* testUser = "testUser";
    const char* testPass = "P@ssword123!";
    registerUser(testUser, testPass);
    printf("\n--- Running Test Cases ---\n");

    // Test Case 1: Successful Login
    printf("Test Case 1: Correct username and password.\n");
    char* result1 = loginUser(testUser, testPass);
    printf("Result: %s\n", result1);
    free(result1);
    printf("--------------------------\n");

    // Test Case 2: Incorrect Password
    printf("Test Case 2: Correct username, incorrect password.\n");
    char* result2 = loginUser(testUser, "WrongPassword");
    printf("Result: %s\n", result2);
    free(result2);
    printf("--------------------------\n");

    // Test Case 3: Incorrect Username
    printf("Test Case 3: Incorrect username.\n");
    char* result3 = loginUser("nonexistentUser", testPass);
    printf("Result: %s\n", result3);
    free(result3);
    printf("--------------------------\n");
    
    // Test Case 4: Case-sensitive Username
    printf("Test Case 4: Case-sensitive username check (should fail).\n");
    char* result4 = loginUser("testuser", testPass);
    printf("Result: %s\n", result4);
    free(result4);
    printf("--------------------------\n");

    // Test Case 5: Empty Password
    printf("Test Case 5: Correct username, empty password (should fail).\n");
    char* result5 = loginUser(testUser, "");
    printf("Result: %s\n", result5);
    free(result5);
    printf("--------------------------\n");

    return 0;
}