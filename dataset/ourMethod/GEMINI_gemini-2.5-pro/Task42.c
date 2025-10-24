#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// Constants
#define SALT_SIZE 16
#define HASH_SIZE 32 // Corresponds to SHA-256
#define PBKDF2_ITERATIONS 250000
#define USER_DB_FILE "users.db"

#define MAX_USERNAME_LEN 50
#define MAX_PASSWORD_LEN 100
#define HEX_SALT_LEN (SALT_SIZE * 2)
#define HEX_HASH_LEN (HASH_SIZE * 2)
#define MAX_LINE_LEN (MAX_USERNAME_LEN + 1 + HEX_SALT_LEN + 1 + HEX_HASH_LEN + 2)

// Converts binary data to a hex string.
// Caller must ensure out_hex has enough space (len * 2 + 1).
bool to_hex(const unsigned char *data, size_t len, char *out_hex) {
    if (data == NULL || out_hex == NULL) return false;
    for (size_t i = 0; i < len; i++) {
        if (snprintf(out_hex + (i * 2), 3, "%02x", data[i]) < 2) {
            return false;
        }
    }
    out_hex[len * 2] = '\0';
    return true;
}

// Converts a hex string to binary data.
// Caller must ensure out_data has enough space (strlen(hex) / 2).
bool from_hex(const char *hex, unsigned char *out_data, size_t out_len) {
    if (hex == NULL || out_data == NULL) return false;
    size_t hex_len = strlen(hex);
    if (hex_len % 2 != 0 || hex_len / 2 != out_len) {
        return false;
    }
    for (size_t i = 0; i < out_len; i++) {
        if (sscanf(hex + 2 * i, "%2hhx", &out_data[i]) != 1) {
            return false;
        }
    }
    return true;
}

// Generates a cryptographically secure random salt
bool generate_salt(unsigned char *salt, size_t len) {
    if (salt == NULL) return false;
    return RAND_bytes(salt, (int)len) == 1;
}

// Hashes a password with a given salt using PBKDF2-HMAC-SHA256
bool hash_password(const char *password, const unsigned char *salt, unsigned char *hash) {
    if (password == NULL || salt == NULL || hash == NULL) return false;
    int result = PKCS5_PBKDF2_HMAC(
        password,
        (int)strlen(password),
        salt,
        SALT_SIZE,
        PBKDF2_ITERATIONS,
        EVP_sha256(),
        HASH_SIZE,
        hash
    );
    return result == 1;
}

// Checks if a user exists in the database
bool user_exists(const char *username) {
    FILE *user_file = fopen(USER_DB_FILE, "r");
    if (user_file == NULL) {
        return false;
    }

    char line[MAX_LINE_LEN];
    bool found = false;
    while (fgets(line, sizeof(line), user_file) != NULL) {
        char *colon_pos = strchr(line, ':');
        if (colon_pos != NULL) {
            size_t username_len = strlen(username);
            if ((size_t)(colon_pos - line) == username_len && strncmp(line, username, username_len) == 0) {
                found = true;
                break;
            }
        }
    }
    fclose(user_file);
    return found;
}

// Registers a new user
bool register_user(const char *username, char *password) {
    if (username == NULL || password == NULL ||
        strlen(username) == 0 || strlen(username) > MAX_USERNAME_LEN ||
        strlen(password) == 0 || strlen(password) > MAX_PASSWORD_LEN) {
        fprintf(stderr, "Error: Invalid username or password length.\n");
        return false;
    }
    if (strchr(username, ':') != NULL) {
        fprintf(stderr, "Error: Username cannot contain ':'.\n");
        return false;
    }

    if (user_exists(username)) {
        fprintf(stderr, "Error: User '%s' already exists.\n", username);
        return false;
    }

    unsigned char salt[SALT_SIZE];
    if (!generate_salt(salt, SALT_SIZE)) {
        fprintf(stderr, "Error: Failed to generate salt.\n");
        return false;
    }

    unsigned char hash[HASH_SIZE];
    if (!hash_password(password, salt, hash)) {
        fprintf(stderr, "Error: Failed to hash password.\n");
        OPENSSL_cleanse(password, strlen(password));
        return false;
    }
    OPENSSL_cleanse(password, strlen(password));

    char salt_hex[HEX_SALT_LEN + 1];
    char hash_hex[HEX_HASH_LEN + 1];
    if (!to_hex(salt, SALT_SIZE, salt_hex) || !to_hex(hash, HASH_SIZE, hash_hex)) {
        fprintf(stderr, "Error: Failed to convert binary to hex.\n");
        return false;
    }

    FILE *user_file = fopen(USER_DB_FILE, "a");
    if (user_file == NULL) {
        perror("Error opening user database for writing");
        return false;
    }

    fprintf(user_file, "%s:%s:%s\n", username, salt_hex, hash_hex);
    fclose(user_file);

    return true;
}

// Authenticates a user
bool authenticate_user(const char *username, char *password) {
    if (username == NULL || password == NULL ||
        strlen(username) > MAX_USERNAME_LEN || strlen(password) > MAX_PASSWORD_LEN) {
        if (password) OPENSSL_cleanse(password, strlen(password));
        return false;
    }
    
    FILE *user_file = fopen(USER_DB_FILE, "r");
    if (user_file == NULL) {
        OPENSSL_cleanse(password, strlen(password));
        return false;
    }

    char line[MAX_LINE_LEN];
    bool user_found = false;
    bool authenticated = false;

    while (fgets(line, sizeof(line), user_file) != NULL) {
        char* newline = strchr(line, '\n');
        if (newline) *newline = '\0';

        char line_copy[MAX_LINE_LEN];
        strncpy(line_copy, line, MAX_LINE_LEN - 1);
        line_copy[MAX_LINE_LEN - 1] = '\0';
        
        char *p = line_copy;
        char *stored_username = strsep(&p, ":");
        if (stored_username == NULL) continue;
        
        if (strcmp(stored_username, username) == 0) {
            user_found = true;
            char *salt_hex = strsep(&p, ":");
            char *hash_hex = strsep(&p, ":");

            if (salt_hex == NULL || hash_hex == NULL) break;

            unsigned char salt[SALT_SIZE];
            unsigned char stored_hash[HASH_SIZE];

            if (!from_hex(salt_hex, salt, SALT_SIZE) || !from_hex(hash_hex, stored_hash, HASH_SIZE)) break;

            unsigned char computed_hash[HASH_SIZE];
            if (!hash_password(password, salt, computed_hash)) break;

            if (CRYPTO_memcmp(stored_hash, computed_hash, HASH_SIZE) == 0) {
                authenticated = true;
            }
            break;
        }
    }
    
    fclose(user_file);
    OPENSSL_cleanse(password, strlen(password));
    return user_found && authenticated;
}


void run_tests() {
    remove(USER_DB_FILE);

    printf("--- Running C Authentication Tests ---\n");

    printf("Test 1: Successful registration... ");
    char pass1[] = "StrongP@ssw0rd123";
    if(register_user("testuser1", pass1)) printf("PASSED\n"); else printf("FAILED\n");

    printf("Test 2: Successful login... ");
    char pass2[] = "StrongP@ssw0rd123";
    if(authenticate_user("testuser1", pass2)) printf("PASSED\n"); else printf("FAILED\n");

    printf("Test 3: Failed login (wrong password)... ");
    char pass3[] = "WrongPassword";
    if(!authenticate_user("testuser1", pass3)) printf("PASSED\n"); else printf("FAILED\n");

    printf("Test 4: Failed login (user not found)... ");
    char pass4[] = "any_password";
    if(!authenticate_user("nonexistent_user", pass4)) printf("PASSED\n"); else printf("FAILED\n");

    printf("Test 5: Failed registration (user already exists)... ");
    char pass5[] = "AnotherPassword";
    if(!register_user("testuser1", pass5)) printf("PASSED\n"); else printf("FAILED\n");

    remove(USER_DB_FILE);
    printf("--- C Tests Finished ---\n\n");
}

int main() {
    run_tests();
    return 0;
}