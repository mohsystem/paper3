#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/crypto.h> // For CRYPTO_memcmp and OPENSSL_cleanse

// NOTE: This code requires OpenSSL. Compile with:
// gcc task122.c -o task122 -lssl -lcrypto

// Configuration constants
#define ITERATIONS 210000
#define KEY_LENGTH_BYTES 32
#define SALT_LENGTH_BYTES 16
#define MIN_PASSWORD_LENGTH 8

// Base64 encode helper
// Caller must free the returned string
char* base64_encode(const unsigned char* input, int length) {
    BIO *b64, *mem;
    BUF_MEM *bptr;
    char *buf;

    mem = BIO_new(BIO_s_mem());
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    mem = BIO_push(b64, mem);

    if (BIO_write(mem, input, length) <= 0) {
        BIO_free_all(mem);
        return NULL;
    }
    BIO_flush(mem);

    BIO_get_mem_ptr(mem, &bptr);
    buf = (char *)malloc(bptr->length + 1);
    if (!buf) {
        BIO_free_all(mem);
        return NULL;
    }
    memcpy(buf, bptr->data, bptr->length);
    buf[bptr->length] = '\0';

    BIO_free_all(mem);
    return buf;
}

// Base64 decode helper
// Caller must free the returned buffer. *output_len will contain the decoded size.
unsigned char* base64_decode(const char* input, int* output_len) {
    BIO *b64, *mem;
    int input_len = strlen(input);
    unsigned char* buffer = (unsigned char*)malloc(input_len);
    if (!buffer) return NULL;

    mem = BIO_new_mem_buf(input, -1);
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    mem = BIO_push(b64, mem);

    *output_len = BIO_read(mem, buffer, input_len);
    BIO_free_all(mem);

    if (*output_len < 0) {
        free(buffer);
        return NULL;
    }
    return buffer;
}


// Hashes a password using PBKDF2-HMAC-SHA256
// Caller is responsible for freeing the returned string
char* hashPassword(const char* password) {
    if (strlen(password) < MIN_PASSWORD_LENGTH) {
        fprintf(stderr, "Error: Password does not meet the length requirement.\n");
        return NULL;
    }

    unsigned char salt[SALT_LENGTH_BYTES];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error generating salt.\n");
        return NULL;
    }

    unsigned char hash[KEY_LENGTH_BYTES];
    if (PKCS5_PBKDF2_HMAC(password, strlen(password),
                           salt, sizeof(salt),
                           ITERATIONS, EVP_sha256(),
                           sizeof(hash), hash) != 1) {
        fprintf(stderr, "Error in PBKDF2 HMAC.\n");
        return NULL;
    }

    char* salt_b64 = base64_encode(salt, sizeof(salt));
    char* hash_b64 = base64_encode(hash, sizeof(hash));
    if (!salt_b64 || !hash_b64) {
        free(salt_b64);
        free(hash_b64);
        return NULL;
    }

    // Allocate buffer for the final formatted string
    size_t result_len = snprintf(NULL, 0, "$pbkdf2-sha256$%d$%s$%s", ITERATIONS, salt_b64, hash_b64) + 1;
    char* result = malloc(result_len);
    if (result) {
        snprintf(result, result_len, "$pbkdf2-sha256$%d$%s$%s", ITERATIONS, salt_b64, hash_b64);
    }
    
    free(salt_b64);
    free(hash_b64);
    OPENSSL_cleanse(hash, sizeof(hash));

    return result;
}

// Verifies a password against a stored hash
bool verifyPassword(const char* password, const char* storedHash) {
    char* hash_copy = strdup(storedHash);
    if (!hash_copy) return false;

    char* saveptr;
    strtok_r(hash_copy, "$", &saveptr); // Consume first empty part
    
    char* algorithm = strtok_r(NULL, "$", &saveptr);
    char* iter_str = strtok_r(NULL, "$", &saveptr);
    char* salt_b64 = strtok_r(NULL, "$", &saveptr);
    char* hash_b64 = strtok_r(NULL, "$", &saveptr);

    if (!algorithm || !iter_str || !salt_b64 || !hash_b64) {
        free(hash_copy);
        return false;
    }

    long iterations = strtol(iter_str, NULL, 10);
    if (iterations <= 0) {
        free(hash_copy);
        return false;
    }

    int salt_len, original_hash_len;
    unsigned char* salt = base64_decode(salt_b64, &salt_len);
    unsigned char* original_hash = base64_decode(hash_b64, &original_hash_len);
    
    free(hash_copy);

    if (!salt || !original_hash) {
        free(salt);
        free(original_hash);
        return false;
    }

    unsigned char comparison_hash[KEY_LENGTH_BYTES];
    int pkcs_ret = PKCS5_PBKDF2_HMAC(password, strlen(password),
                                     salt, salt_len,
                                     iterations, EVP_sha256(),
                                     sizeof(comparison_hash), comparison_hash);
    
    bool result = false;
    if (pkcs_ret == 1) {
        // Constant-time comparison
        if ((size_t)original_hash_len == sizeof(comparison_hash)) {
           result = (CRYPTO_memcmp(original_hash, comparison_hash, original_hash_len) == 0);
        }
    }
    
    free(salt);
    free(original_hash);
    OPENSSL_cleanse(comparison_hash, sizeof(comparison_hash));

    return result;
}

// --- Demo application logic ---
#define MAX_USERS 10
#define MAX_USERNAME_LEN 50
#define MAX_HASH_LEN 256

struct User {
    char username[MAX_USERNAME_LEN];
    char password_hash[MAX_HASH_LEN];
};

struct User userDatabase[MAX_USERS];
int user_count = 0;

void signUp(const char* username, const char* password) {
    printf("Attempting to sign up user: %s\n", username);
    if (user_count >= MAX_USERS) {
        printf("Signup failed: Database is full.\n");
        return;
    }

    char* hashed_password = hashPassword(password);
    if (hashed_password) {
        strncpy(userDatabase[user_count].username, username, MAX_USERNAME_LEN - 1);
        userDatabase[user_count].username[MAX_USERNAME_LEN - 1] = '\0';
        
        strncpy(userDatabase[user_count].password_hash, hashed_password, MAX_HASH_LEN - 1);
        userDatabase[user_count].password_hash[MAX_HASH_LEN - 1] = '\0';
        
        user_count++;
        printf("User '%s' signed up successfully.\n", username);
        free(hashed_password);
    } else {
        printf("Signup for user '%s' failed.\n", username);
    }
}

void login(const char* username, const char* password) {
    printf("Attempting to log in user: %s\n", username);
    char* stored_hash = NULL;
    for (int i = 0; i < user_count; i++) {
        if (strncmp(username, userDatabase[i].username, MAX_USERNAME_LEN) == 0) {
            stored_hash = userDatabase[i].password_hash;
            break;
        }
    }

    if (stored_hash == NULL) {
        printf("Login failed: User not found.\n");
        return;
    }

    if (verifyPassword(password, stored_hash)) {
        printf("Login successful for user '%s'.\n", username);
    } else {
        printf("Login failed: Incorrect password.\n");
    }
}

int main() {
    // Test Case 1: Successful signup and login
    printf("--- Test Case 1 ---\n");
    const char* user1 = "alice";
    const char* pass1 = "Str0ngP@ssw0rd1";
    signUp(user1, pass1);
    login(user1, pass1);
    printf("\n");

    // Test Case 2: Login with incorrect password
    printf("--- Test Case 2 ---\n");
    const char* wrongPass1 = "WrongPassword!_";
    login(user1, wrongPass1);
    printf("\n");

    // Test Case 3: Signup with a weak (short) password
    printf("--- Test Case 3 ---\n");
    const char* user2 = "bob";
    const char* pass2_weak = "short";
    signUp(user2, pass2_weak);
    printf("\n");

    // Test Case 4: Successful signup for a second user
    printf("--- Test Case 4 ---\n");
    const char* user3 = "charlie";
    const char* pass3 = "An0th3rS3curePwd";
    signUp(user3, pass3);
    login(user3, pass3);
    printf("\n");

    // Test Case 5: Attempt to log in a non-existent user
    printf("--- Test Case 5 ---\n");
    login("david", "SomePassword123");
    printf("\n");

    return 0;
}