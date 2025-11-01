#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// To compile: gcc your_file_name.c -o Lse95 -lssl -lcrypto
// You must have OpenSSL development libraries installed.

#define SALT_LENGTH 16
#define MAX_USERS 100 // For demonstration purposes

typedef struct {
    char* username;
    unsigned char* salt;
    unsigned char* hashedPassword;
    size_t hashLength;
} UserData;

// In-memory user database (a simple array for demonstration)
static UserData userDatabase[MAX_USERS];
static int userCount = 0;

/**
 * @brief Hashes a password with a given salt using SHA-256.
 * @param password The password string.
 * @param salt The salt buffer.
 * @param salt_len The length of the salt.
 * @param hash_buf Buffer to store the resulting hash.
 * @param hash_len Pointer to store the length of the resulting hash.
 * @return 1 on success, 0 on failure.
 */
int hashPassword(const char* password, const unsigned char* salt, size_t salt_len, unsigned char* hash_buf, unsigned int* hash_len) {
    EVP_MD_CTX* mdctx;
    const EVP_MD* md = EVP_sha256();

    if ((mdctx = EVP_MD_CTX_new()) == NULL) return 0;
    if (EVP_DigestInit_ex(mdctx, md, NULL) != 1) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }
    // Combine salt and password for hashing
    if (EVP_DigestUpdate(mdctx, salt, salt_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }
    if (EVP_DigestUpdate(mdctx, password, strlen(password)) != 1) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }
    if (EVP_DigestFinal_ex(mdctx, hash_buf, hash_len) != 1) {
        EVP_MD_CTX_free(mdctx);
        return 0;
    }

    EVP_MD_CTX_free(mdctx);
    return 1;
}

/**
 * @brief Registers a new user.
 * @param username The username.
 * @param password The password.
 * @return 1 if registration is successful, 0 if user already exists or db is full.
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

    // Generate salt
    unsigned char* salt = (unsigned char*)malloc(SALT_LENGTH);
    if (!salt || RAND_bytes(salt, SALT_LENGTH) != 1) {
        printf("Error generating salt.\n");
        free(salt);
        return 0;
    }

    // Hash password
    unsigned char* hash_buf = (unsigned char*)malloc(EVP_MAX_MD_SIZE);
    unsigned int hash_len;
    if (!hash_buf || !hashPassword(password, salt, SALT_LENGTH, hash_buf, &hash_len)) {
        printf("Error hashing password.\n");
        free(salt);
        free(hash_buf);
        return 0;
    }

    // Store user data
    userDatabase[userCount].username = strdup(username);
    userDatabase[userCount].salt = salt;
    userDatabase[userCount].hashedPassword = hash_buf;
    userDatabase[userCount].hashLength = hash_len;
    userCount++;

    printf("User '%s' registered successfully.\n", username);
    return 1;
}

/**
 * @brief Verifies a user's login credentials.
 * @param username The username.
 * @param password The password to verify.
 * @return 1 if credentials are correct, 0 otherwise.
 */
int loginUser(const char* username, const char* password) {
    UserData* user = NULL;
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(userDatabase[i].username, username) == 0) {
            user = &userDatabase[i];
            break;
        }
    }

    if (user == NULL) {
        printf("Login failed: User '%s' not found.\n", username);
        return 0;
    }

    unsigned char provided_hash[EVP_MAX_MD_SIZE];
    unsigned int provided_hash_len;

    if (!hashPassword(password, user->salt, SALT_LENGTH, provided_hash, &provided_hash_len)) {
        printf("Login failed: Error during password verification.\n");
        return 0;
    }

    // Constant-time comparison to prevent timing attacks
    if (user->hashLength == provided_hash_len && CRYPTO_memcmp(user->hashedPassword, provided_hash, user->hashLength) == 0) {
        printf("Login successful for user '%s'.\n", username);
        return 1;
    } else {
        printf("Login failed: Incorrect password for user '%s'.\n", username);
        return 0;
    }
}

/**
 * @brief Frees all dynamically allocated memory.
 */
void cleanup() {
    for (int i = 0; i < userCount; ++i) {
        free(userDatabase[i].username);
        free(userDatabase[i].salt);
        free(userDatabase[i].hashedPassword);
    }
}

void Lse95() {
    printf("--- Running Test Cases ---\n");

    // Test Case 1: Register a new user
    printf("\n[Test Case 1: Successful Registration]\n");
    registerUser("alice", "Password123!");

    // Test Case 2: Successful Login
    printf("\n[Test Case 2: Successful Login]\n");
    loginUser("alice", "Password123!");

    // Test Case 3: Failed Login (Wrong Password)
    printf("\n[Test Case 3: Failed Login - Wrong Password]\n");
    loginUser("alice", "WrongPassword");

    // Test Case 4: Failed Registration (Username already exists)
    printf("\n[Test Case 4: Failed Registration - User Exists]\n");
    registerUser("alice", "AnotherPassword");

    // Test Case 5: Failed Login (User does not exist)
    printf("\n[Test Case 5: Failed Login - User Not Found]\n");
    loginUser("bob", "Password123!");

    printf("\n--- Test Cases Finished ---\n");

    // Clean up allocated memory before exiting
    cleanup();
}


int main() {
    Lse95();
    return 0;
}