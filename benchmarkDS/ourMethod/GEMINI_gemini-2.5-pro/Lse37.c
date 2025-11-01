#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp

// Rule #16: For clearing sensitive data from memory.
#ifdef __STDC_LIB_EXT1__
#define SECURE_ZERO_MEMORY(p, s) memset_s(p, s, 0, s)
#else
static void secure_zero_memory(void *p, size_t s) {
    volatile unsigned char *v = (volatile unsigned char *)p;
    while (s--) {
        *v++ = 0;
    }
}
#define SECURE_ZERO_MEMORY(p, s) secure_zero_memory(p, s)
#endif

#define ITERATIONS 600000
#define SALT_SIZE 16
#define KEY_LENGTH 32 // 256 bits
#define MAX_USERNAME_LEN 100
#define MAX_PASSWORD_LEN 100
#define MAX_USERS 10

typedef struct {
    char username[MAX_USERNAME_LEN + 1];
    unsigned char salt[SALT_SIZE];
    unsigned char hashedPassword[KEY_LENGTH];
} UserCredentials;

// A simple array to act as our "database".
static UserCredentials userDatabase[MAX_USERS];
static int userCount = 0;

void cleanup_openssl() {
    ERR_free_strings();
    EVP_cleanup();
}

// Returns 0 on success, -1 on failure.
int hash_password(const char* password, const unsigned char* salt, unsigned char* hash_out) {
    if (!password || !salt || !hash_out) return -1;
    int result = PKCS5_PBKDF2_HMAC(
        password,
        strlen(password),
        salt,
        SALT_SIZE,
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        hash_out
    );
    return (result == 1) ? 0 : -1;
}

// Returns 0 on success, -1 on failure.
int setup_database() {
    if (userCount >= MAX_USERS) return -1;

    const char* username = "admin";
    const char* password = "password123";

    strncpy(userDatabase[userCount].username, username, MAX_USERNAME_LEN);
    userDatabase[userCount].username[MAX_USERNAME_LEN] = '\0';

    if (RAND_bytes(userDatabase[userCount].salt, SALT_SIZE) != 1) {
        fprintf(stderr, "Failed to generate random salt.\n");
        return -1;
    }
    
    if (hash_password(password, userDatabase[userCount].salt, userDatabase[userCount].hashedPassword) != 0) {
        fprintf(stderr, "Failed to hash password during setup.\n");
        return -1;
    }
    userCount++;
    
    if (userCount < MAX_USERS) {
        strncpy(userDatabase[userCount].username, "error_user", MAX_USERNAME_LEN);
        userDatabase[userCount].username[MAX_USERNAME_LEN] = '\0';
        memset(userDatabase[userCount].salt, 0, SALT_SIZE); // Simulate corrupted data
        memset(userDatabase[userCount].hashedPassword, 0, KEY_LENGTH);
        userCount++;
    }
    return 0;
}

const char* login(const char* username, const char* password) {
    if (username == NULL || password == NULL || username[0] == '\0' || password[0] == '\0') {
        return "cant_login";
    }
    size_t user_len = strnlen(username, MAX_USERNAME_LEN + 1);
    size_t pass_len = strnlen(password, MAX_PASSWORD_LEN + 1);
    if (user_len > MAX_USERNAME_LEN || pass_len > MAX_PASSWORD_LEN) {
        return "cant_login";
    }
    
    UserCredentials* storedCreds = NULL;
    for (int i = 0; i < userCount; ++i) {
        if (strncmp(userDatabase[i].username, username, MAX_USERNAME_LEN) == 0) {
            storedCreds = &userDatabase[i];
            break;
        }
    }

    if (storedCreds == NULL) {
        return "cant_login";
    }

    int is_salt_zeroed = 1;
    for(int i = 0; i < SALT_SIZE; ++i) {
        if (storedCreds->salt[i] != 0) {
            is_salt_zeroed = 0;
            break;
        }
    }
    if (is_salt_zeroed) {
        return "internal_server_error";
    }

    unsigned char providedPasswordHash[KEY_LENGTH] = {0};
    if (hash_password(password, storedCreds->salt, providedPasswordHash) != 0) {
        SECURE_ZERO_MEMORY(providedPasswordHash, KEY_LENGTH);
        return "internal_server_error";
    }

    int cmp_result = CRYPTO_memcmp(storedCreds->hashedPassword, providedPasswordHash, KEY_LENGTH);
    SECURE_ZERO_MEMORY(providedPasswordHash, KEY_LENGTH);

    if (cmp_result == 0) {
        return "success";
    } else {
        return "cant_login";
    }
}

void run_tests() {
    printf("--- 5 Test Cases ---\n");

    // 1. Correct login
    printf("Test 1 (Correct login): %s\n", login("admin", "password123"));

    // 2. Incorrect password
    printf("Test 2 (Incorrect password): %s\n", login("admin", "wrongpassword"));

    // 3. Incorrect username
    printf("Test 3 (Incorrect username): %s\n", login("unknownuser", "password123"));

    // 4. Empty input
    printf("Test 4 (Empty input): %s\n", login("", ""));

    // 5. Internal error simulation
    printf("Test 5 (Internal error simulation): %s\n", login("error_user", "any_password"));
}

int main(void) {
    OpenSSL_add_all_algorithms();
    ERR_load_crypto_strings();
    atexit(cleanup_openssl);

    if (setup_database() != 0) {
        fprintf(stderr, "Failed to initialize the database. Exiting.\n");
        return 1;
    }
    
    run_tests();
    
    return 0;
}