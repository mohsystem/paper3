
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#define ITERATIONS 210000   /* PBKDF2 iterations >= 210000 for security */
#define KEY_LENGTH 32       /* 32 bytes = 256 bits */
#define SALT_LENGTH 16      /* 16-byte salt */
#define MAX_USERNAME_LEN 256
#define MAX_PASSWORD_LEN 256

/**
 * Converts binary data to hexadecimal string
 * Caller must free the returned string
 */
char* toHex(const unsigned char* data, size_t len) {
    const char* hex = "0123456789abcdef";
    char* result = (char*)malloc(len * 2 + 1);
    if (result == NULL) {
        return NULL;
    }
    
    for (size_t i = 0; i < len; i++) {
        result[i * 2] = hex[(data[i] >> 4) & 0xF];
        result[i * 2 + 1] = hex[data[i] & 0xF];
    }
    result[len * 2] = '\\0';
    return result;
}

/**
 * Registers a user by hashing their password with a unique salt.
 * 
 * Returns: A dynamically allocated string containing salt:hash (caller must free)
 * Returns NULL on error
 */
char* registerUser(const char* username, const char* password) {
    unsigned char salt[SALT_LENGTH];
    unsigned char hash[KEY_LENGTH];
    char* saltHex = NULL;
    char* hashHex = NULL;
    char* result = NULL;
    size_t usernameLen;
    size_t passwordLen;
    size_t i;
    int isWhitespace;
    
    /* Validate inputs - check for NULL */
    if (username == NULL || password == NULL) {
        fprintf(stderr, "Error: Username or password is NULL\\n");
        return NULL;
    }
    
    /* Validate username - check length and non-empty */
    usernameLen = strlen(username);
    if (usernameLen == 0 || usernameLen >= MAX_USERNAME_LEN) {
        fprintf(stderr, "Error: Invalid username length\\n");
        return NULL;
    }
    
    /* Check username is not only whitespace */
    isWhitespace = 1;
    for (i = 0; i < usernameLen; i++) {
        if (username[i] != ' ' && username[i] != '\\t' && 
            username[i] != '\\n' && username[i] != '\\r') {
            isWhitespace = 0;
            break;
        }
    }
    if (isWhitespace) {
        fprintf(stderr, "Error: Username cannot be empty\\n");
        return NULL;
    }
    
    /* Validate password length */
    passwordLen = strlen(password);
    if (passwordLen < 8 || passwordLen >= MAX_PASSWORD_LEN) {
        fprintf(stderr, "Error: Password must be at least 8 characters\\n");
        return NULL;
    }
    
    /* Generate a cryptographically secure random salt (unique per user) */
    if (RAND_bytes(salt, SALT_LENGTH) != 1) {
        fprintf(stderr, "Error: Failed to generate random salt\\n");
        return NULL;
    }
    
    /* Hash the password using PBKDF2-HMAC-SHA256 */
    if (PKCS5_PBKDF2_HMAC(password, (int)passwordLen, salt, SALT_LENGTH,
                          ITERATIONS, EVP_sha256(), KEY_LENGTH, hash) != 1) {
        fprintf(stderr, "Error: Password hashing failed\\n");
        return NULL;
    }
    
    /* Convert salt and hash to hex */
    saltHex = toHex(salt, SALT_LENGTH);
    if (saltHex == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    hashHex = toHex(hash, KEY_LENGTH);
    if (hashHex == NULL) {
        free(saltHex);
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    /* Allocate result string for "salt:hash" */
    result = (char*)malloc(strlen(saltHex) + strlen(hashHex) + 2);
    if (result == NULL) {
        free(saltHex);
        free(hashHex);
        fprintf(stderr, "Error: Memory allocation failed\\n");
        return NULL;
    }
    
    /* Format result as salt:hash */
    sprintf(result, "%s:%s", saltHex, hashHex);
    
    /* Clear sensitive data from memory using explicit_bzero or memset_s */
#ifdef __STDC_LIB_EXT1__
    memset_s((void*)password, passwordLen, 0, passwordLen);
#else
    /* Portable explicit zero - use volatile to prevent optimization */
    volatile unsigned char* p = (volatile unsigned char*)password;
    for (i = 0; i < passwordLen; i++) {
        p[i] = 0;
    }
#endif
    
    /* Clean up */
    free(saltHex);
    free(hashHex);
    
    return result;
}

int main(void) {
    char* result;
    
    /* Test case 1: Valid username and password */
    result = registerUser("alice", "SecurePass123!");
    if (result != NULL) {
        printf("User 'alice' registered. Hash: %.50s...\\n", result);
        free(result);
    }
    
    /* Test case 2: Different user with different password */
    result = registerUser("bob", "AnotherP@ss456");
    if (result != NULL) {
        printf("User 'bob' registered. Hash: %.50s...\\n", result);
        free(result);
    }
    
    /* Test case 3: Same password for different user should produce different hash */
    result = registerUser("charlie", "SecurePass123!");
    if (result != NULL) {
        printf("User 'charlie' registered. Hash: %.50s...\\n", result);
        printf("Note: Same password produces different hash due to unique salt\\n");
        free(result);
    }
    
    /* Test case 4: Minimum password length */
    result = registerUser("dave", "MinPass8");
    if (result != NULL) {
        printf("User 'dave' registered. Hash: %.50s...\\n", result);
        free(result);
    }
    
    /* Test case 5: Long username and password */
    result = registerUser("elizabeth_longusername", "VeryLongP@ssw0rd!WithSpecialChars");
    if (result != NULL) {
        printf("User 'elizabeth_longusername' registered. Hash: %.50s...\\n", result);
        free(result);
    }
    
    return 0;
}
