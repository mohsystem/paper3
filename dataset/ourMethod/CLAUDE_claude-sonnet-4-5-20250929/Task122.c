
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

/* OpenSSL headers for cryptographic operations */
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

/* Security constants following OWASP guidelines */
#define SALT_SIZE 16
#define HASH_SIZE 32
#define PBKDF2_ITERATIONS 210000
#define MIN_PASSWORD_LENGTH 8
#define MAX_PASSWORD_LENGTH 128
#define MAX_USERNAME_LENGTH 64
#define MAX_LINE_LENGTH 512

/* Secure memory cleanup wrapper */
static void secure_clear(void* ptr, size_t size) {
    if (ptr != NULL && size > 0) {
        OPENSSL_cleanse(ptr, size);
    }
}

/* Validate username: alphanumeric and underscore only */
static bool validate_username(const char* username) {
    size_t len;
    size_t i;
    
    if (username == NULL) {
        return false;
    }
    
    len = strlen(username);
    if (len == 0 || len > MAX_USERNAME_LENGTH) {
        return false;
    }
    
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i]) && username[i] != '_') {
            return false;
        }
    }
    
    return true;
}

/* Validate password strength and length */
static bool validate_password(const char* password) {
    size_t len;
    size_t i;
    bool has_upper = false;
    bool has_lower = false;
    bool has_digit = false;
    bool has_special = false;
    int complexity;
    
    if (password == NULL) {
        return false;
    }
    
    len = strlen(password);
    if (len < MIN_PASSWORD_LENGTH || len > MAX_PASSWORD_LENGTH) {
        return false;
    }
    
    for (i = 0; i < len; i++) {
        if (isupper((unsigned char)password[i])) {
            has_upper = true;
        } else if (islower((unsigned char)password[i])) {
            has_lower = true;
        } else if (isdigit((unsigned char)password[i])) {
            has_digit = true;
        } else {
            has_special = true;
        }
    }
    
    complexity = (has_upper ? 1 : 0) + (has_lower ? 1 : 0) + 
                 (has_digit ? 1 : 0) + (has_special ? 1 : 0);
    
    return complexity >= 3;
}

/* Generate cryptographically secure random salt */
static bool generate_salt(unsigned char* salt, size_t size) {
    if (salt == NULL || size == 0) {
        return false;
    }
    
    /* Use OpenSSL CSPRNG for secure randomness */
    if (RAND_bytes(salt, (int)size) != 1) {
        return false;
    }
    
    return true;
}

/* Derive key using PBKDF2-HMAC-SHA256 */
static bool derive_key(const char* password, const unsigned char* salt,
                       size_t salt_size, unsigned char* hash, size_t hash_size) {
    int result;
    
    if (password == NULL || salt == NULL || hash == NULL ||
        salt_size == 0 || hash_size == 0) {
        return false;
    }
    
    /* High iteration count protects against brute force */
    result = PKCS5_PBKDF2_HMAC(
        password,
        (int)strlen(password),
        salt,
        (int)salt_size,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        (int)hash_size,
        hash
    );
    
    return result == 1;
}

/* Convert binary to hex string - caller must free returned string */
static char* to_hex(const unsigned char* data, size_t size) {
    char* hex;
    size_t i;
    const char hex_chars[] = "0123456789abcdef";
    
    if (data == NULL || size == 0) {
        return NULL;
    }
    
    /* Allocate space for hex string plus null terminator */
    hex = (char*)malloc(size * 2 + 1);
    if (hex == NULL) {
        return NULL;
    }
    
    for (i = 0; i < size; i++) {
        hex[i * 2] = hex_chars[(data[i] >> 4) & 0x0F];
        hex[i * 2 + 1] = hex_chars[data[i] & 0x0F];
    }
    hex[size * 2] = '\\0';
    
    return hex;
}

/* Check if username already exists in file */
static bool user_exists(const char* username, const char* filename) {
    FILE* file = NULL;
    char line[MAX_LINE_LENGTH];
    char* colon_pos;
    size_t username_len;
    bool exists = false;
    
    if (username == NULL || filename == NULL) {
        return false;
    }
    
    file = fopen(filename, "r");
    if (file == NULL) {
        return false; /* File doesn't exist yet */\n    }\n    \n    username_len = strlen(username);\n    \n    while (fgets(line, sizeof(line), file) != NULL) {\n        /* Ensure null termination */\n        line[sizeof(line) - 1] = '\\0';\n        \n        /* Find first colon separator */\n        colon_pos = strchr(line, ':');
        if (colon_pos != NULL) {
            size_t stored_user_len = (size_t)(colon_pos - line);
            
            /* Compare username lengths and content */
            if (stored_user_len == username_len &&
                memcmp(line, username, username_len) == 0) {
                exists = true;
                break;
            }
        }
    }
    
    fclose(file);
    return exists;
}

/* Store user credentials securely */
static bool store_user(const char* username, const char* password,
                       const char* filename) {
    unsigned char salt[SALT_SIZE];
    unsigned char hash[HASH_SIZE];
    char* salt_hex = NULL;
    char* hash_hex = NULL;
    FILE* file = NULL;
    bool success = false;
    
    /* Initialize buffers */
    memset(salt, 0, sizeof(salt));
    memset(hash, 0, sizeof(hash));
    
    /* Validate all inputs - fail closed on error */
    if (username == NULL || password == NULL || filename == NULL) {
        fprintf(stderr, "Invalid input parameters\\n");
        goto cleanup;
    }
    
    if (!validate_username(username)) {
        fprintf(stderr, "Invalid username format\\n");
        goto cleanup;
    }
    
    if (!validate_password(password)) {
        fprintf(stderr, "Password does not meet complexity requirements\\n");
        goto cleanup;
    }
    
    /* Check for duplicate username */
    if (user_exists(username, filename)) {
        fprintf(stderr, "Username already exists\\n");
        goto cleanup;
    }
    
    /* Generate unique random salt */
    if (!generate_salt(salt, SALT_SIZE)) {
        fprintf(stderr, "Failed to generate salt\\n");
        goto cleanup;
    }
    
    /* Derive password hash */
    if (!derive_key(password, salt, SALT_SIZE, hash, HASH_SIZE)) {
        fprintf(stderr, "Failed to derive key\\n");
        goto cleanup;
    }
    
    /* Convert to hex for storage */
    salt_hex = to_hex(salt, SALT_SIZE);
    hash_hex = to_hex(hash, HASH_SIZE);
    
    if (salt_hex == NULL || hash_hex == NULL) {
        fprintf(stderr, "Failed to convert to hex\\n");
        goto cleanup;
    }
    
    /* Open file for appending */
    file = fopen(filename, "a");
    if (file == NULL) {
        fprintf(stderr, "Failed to open storage file\\n");
        goto cleanup;
    }
    
    /* Write in format: username:salt:hash */
    if (fprintf(file, "%s:%s:%s\\n", username, salt_hex, hash_hex) < 0) {
        fprintf(stderr, "Failed to write to storage file\\n");
        goto cleanup;
    }
    
    success = true;

cleanup:
    /* Secure cleanup of sensitive data */
    secure_clear(salt, sizeof(salt));
    secure_clear(hash, sizeof(hash));
    
    if (salt_hex != NULL) {
        secure_clear(salt_hex, strlen(salt_hex));
        free(salt_hex);
    }
    
    if (hash_hex != NULL) {
        secure_clear(hash_hex, strlen(hash_hex));
        free(hash_hex);
    }
    
    if (file != NULL) {
        fclose(file);
    }
    
    return success;
}

int main(void) {
    const char* storage_file = "users.txt";
    
    printf("=== Secure User Signup System ===\\n");
    printf("Password requirements:\\n");
    printf("- Minimum 8 characters\\n");
    printf("- At least 3 of: uppercase, lowercase, digit, special character\\n");
    printf("\\n");
    
    /* Test case 1: Valid signup */
    {
        printf("\\nTest 1: Valid user signup\\n");
        if (store_user("alice123", "SecureP@ss1", storage_file)) {
            printf("User registered successfully!\\n");
        } else {
            printf("Registration failed\\n");
        }
    }
    
    /* Test case 2: Another valid user */
    {
        printf("\\nTest 2: Another valid user\\n");
        if (store_user("bob_user", "MyStr0ng#Pass", storage_file)) {
            printf("User registered successfully!\\n");
        } else {
            printf("Registration failed\\n");
        }
    }
    
    /* Test case 3: Duplicate username */
    {
        printf("\\nTest 3: Duplicate username attempt\\n");
        if (store_user("alice123", "AnotherP@ss1", storage_file)) {
            printf("User registered successfully!\\n");
        } else {
            printf("Registration failed (expected - duplicate)\\n");
        }
    }
    
    /* Test case 4: Weak password */
    {
        printf("\\nTest 4: Weak password attempt\\n");
        if (store_user("charlie", "weak", storage_file)) {
            printf("User registered successfully!\\n");
        } else {
            printf("Registration failed (expected - weak password)\\n");
        }
    }
    
    /* Test case 5: Invalid username */
    {
        printf("\\nTest 5: Invalid username attempt\\n");
        if (store_user("user@invalid!", "StrongP@ss9", storage_file)) {
            printf("User registered successfully!\\n");
        } else {
            printf("Registration failed (expected - invalid username)\\n");
        }
    }
    
    printf("\\n=== All tests completed ===\\n");
    
    return 0;
}
