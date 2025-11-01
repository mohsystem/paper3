#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>

// Note: This code requires linking with OpenSSL libraries.
// Compile with: gcc your_file.c -o your_app -lssl -lcrypto

#define SALT_LENGTH 16
#define HASH_LENGTH 32 // For SHA-256
#define ITERATIONS 210000
#define HASH_ALGORITHM EVP_sha256()

#define MAX_USERNAME_LEN 20
#define MIN_USERNAME_LEN 3
#define MAX_PASSWORD_LEN 64
#define MIN_PASSWORD_LEN 8
#define MAX_USERS 10

// Rule #15: A function to securely zero memory that is less likely to be optimized away
static void secure_zero_memory(void *v, size_t n) {
    volatile unsigned char *p = v;
    while (n--) {
        *p++ = 0;
    }
}

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    unsigned char salt[SALT_LENGTH];
    unsigned char hashedPassword[HASH_LENGTH];
} UserData;

// Simulate a database using a static array
static UserData userDatabase[MAX_USERS];
static int userCount = 0;

// Rule #1: Input validation for username and password formats and lengths
bool is_alnum_str(const char* str, size_t min_len, size_t max_len) {
    if (str == NULL) return false;
    size_t len = strlen(str);
    if (len < min_len || len > max_len) return false;
    for (size_t i = 0; i < len; i++) {
        if (!((str[i] >= 'a' && str[i] <= 'z') ||
              (str[i] >= 'A' && str[i] <= 'Z') ||
              (str[i] >= '0' && str[i] <= '9'))) {
            return false;
        }
    }
    return true;
}

bool is_valid_password(const char* str, size_t min_len, size_t max_len) {
    if (str == NULL) return false;
    size_t len = strlen(str);
    if (len < min_len || len > max_len) return false;
    bool has_letter = false;
    bool has_digit = false;
    for (size_t i = 0; i < len; i++) {
        if ((str[i] >= 'a' && str[i] <= 'z') || (str[i] >= 'A' && str[i] <= 'Z')) has_letter = true;
        if (str[i] >= '0' && str[i] <= '9') has_digit = true;
    }
    return has_letter && has_digit;
}

bool hash_password(const char* password, const unsigned char* salt, unsigned char* hash) {
    if (password == NULL || salt == NULL || hash == NULL) {
        return false;
    }
    int result = PKCS5_PBKDF2_HMAC(password, strlen(password), salt, SALT_LENGTH, ITERATIONS, HASH_ALGORITHM, HASH_LENGTH, hash);
    return result == 1;
}

void register_user(const char* username, const char* password) {
    if (userCount >= MAX_USERS) {
        fprintf(stderr, "Database is full. Cannot register new user.\n");
        return;
    }
    if (!is_alnum_str(username, 1, MAX_USERNAME_LEN)) {
        fprintf(stderr, "Invalid username for registration.\n");
        return;
    }

    UserData newUser;
    strncpy(newUser.username, username, MAX_USERNAME_LEN);
    newUser.username[MAX_USERNAME_LEN] = '\0'; // Ensure null termination

    if (RAND_bytes(newUser.salt, SALT_LENGTH) != 1) {
        fprintf(stderr, "Failed to generate salt.\n");
        ERR_print_errors_fp(stderr);
        return;
    }

    if (!hash_password(password, newUser.salt, newUser.hashedPassword)) {
        fprintf(stderr, "Failed to hash password.\n");
        ERR_print_errors_fp(stderr);
        return;
    }

    userDatabase[userCount++] = newUser;
    printf("User '%s' registered.\n", username);
}

const char* login(const char* username, const char* password) {
    // Rule #1: Input validation
    if (!is_alnum_str(username, MIN_USERNAME_LEN, MAX_USERNAME_LEN)) {
        fprintf(stderr, "Error: Invalid username format.\n");
        return "LOGIN_PAGE";
    }
    if (!is_valid_password(password, MIN_PASSWORD_LEN, MAX_PASSWORD_LEN)) {
        fprintf(stderr, "Error: Invalid password format.\n");
        return "LOGIN_PAGE";
    }

    UserData* user = NULL;
    for (int i = 0; i < userCount; i++) {
        if (strncmp(userDatabase[i].username, username, MAX_USERNAME_LEN) == 0) {
            user = &userDatabase[i];
            break;
        }
    }

    if (user == NULL) {
        // User not found. To prevent timing attacks, perform a dummy hash.
        unsigned char dummy_salt[SALT_LENGTH];
        unsigned char dummy_hash[HASH_LENGTH];
        if (RAND_bytes(dummy_salt, SALT_LENGTH) == 1) {
            hash_password("dummyPassword", dummy_salt, dummy_hash);
        }
        return "LOGIN_PAGE";
    }
    
    unsigned char providedPasswordHash[HASH_LENGTH];
    if (!hash_password(password, user->salt, providedPasswordHash)) {
        fprintf(stderr, "Error hashing provided password during login.\n");
        ERR_print_errors_fp(stderr);
        secure_zero_memory(providedPasswordHash, HASH_LENGTH);
        return "LOGIN_PAGE";
    }

    // Rule #7: Use constant-time comparison from vetted library
    int diff = CRYPTO_memcmp(user->hashedPassword, providedPasswordHash, HASH_LENGTH);

    // Rule #15: Clear sensitive data from memory after use
    secure_zero_memory(providedPasswordHash, HASH_LENGTH);

    return (diff == 0) ? "HOME_PAGE" : "LOGIN_PAGE";
}

void cleanup_database() {
    secure_zero_memory(userDatabase, sizeof(userDatabase));
}

int main() {
    register_user("admin", "AdminPass123");
    register_user("user1", "UserPass456");

    printf("\n--- Running Test Cases ---\n");

    // Test Case 1: Successful login
    printf("\nTest Case 1: Valid credentials for 'admin'\n");
    const char* result1 = login("admin", "AdminPass123");
    printf("Result: Login %s. Redirecting to %s\n", (strcmp(result1, "HOME_PAGE") == 0 ? "successful" : "failed"), result1);

    // Test Case 2: Incorrect password
    printf("\nTest Case 2: Invalid password for 'admin'\n");
    const char* result2 = login("admin", "WrongPass789");
    printf("Result: Login %s. Redirecting to %s\n", (strcmp(result2, "HOME_PAGE") == 0 ? "successful" : "failed"), result2);

    // Test Case 3: Non-existent user
    printf("\nTest Case 3: Non-existent user 'nouser'\n");
    const char* result3 = login("nouser", "SomePass123");
    printf("Result: Login %s. Redirecting to %s\n", (strcmp(result3, "HOME_PAGE") == 0 ? "successful" : "failed"), result3);

    // Test Case 4: Invalid username format
    printf("\nTest Case 4: Invalid username format 'us'\n");
    const char* result4 = login("us", "SomePass123");
    printf("Result: Login %s. Redirecting to %s\n", (strcmp(result4, "HOME_PAGE") == 0 ? "successful" : "failed"), result4);

    // Test Case 5: Invalid password format (too short)
    printf("\nTest Case 5: Invalid password format for 'user1'\n");
    const char* result5 = login("user1", "pass");
    printf("Result: Login %s. Redirecting to %s\n", (strcmp(result5, "HOME_PAGE") == 0 ? "successful" : "failed"), result5);

    cleanup_database();
    return 0;
}