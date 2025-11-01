#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/err.h>

// Configuration constants
#define ITERATIONS 310000
#define KEY_LENGTH 64 // in bytes (512 bits)
#define SALT_SIZE 16  // in bytes

// Helper to convert byte array to hex string into a provided buffer
void bytes_to_hex(const unsigned char* data, size_t data_len, char* hex_str, size_t hex_str_size) {
    if (hex_str_size < (data_len * 2 + 1)) {
        if (hex_str_size > 0) hex_str[0] = '\0';
        return;
    }
    for (size_t i = 0; i < data_len; ++i) {
        snprintf(hex_str + (i * 2), 3, "%02x", data[i]);
    }
}

// Hashes a password for user registration.
// Returns 0 on success, -1 on failure.
int register_user(const char* username, const char* password, char* output_buffer, size_t output_buffer_size) {
    if (password == NULL || *password == '\0') {
        fprintf(stderr, "Password cannot be null or empty.\n");
        return -1;
    }
    if (username == NULL || *username == '\0') {
        fprintf(stderr, "Username cannot be null or empty.\n");
        return -1;
    }

    size_t required_size = (SALT_SIZE * 2) + 1 + (KEY_LENGTH * 2) + 1;
    if (output_buffer_size < required_size) {
        fprintf(stderr, "Output buffer is too small.\n");
        return -1;
    }

    unsigned char salt[SALT_SIZE];
    if (RAND_bytes(salt, sizeof(salt)) != 1) {
        fprintf(stderr, "Error generating random salt.\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }

    unsigned char hash[KEY_LENGTH];
    
    size_t pass_len = strlen(password);
    char* password_copy = (char*)malloc(pass_len + 1);
    if (!password_copy) {
        fprintf(stderr, "Failed to allocate memory for password copy.\n");
        return -1;
    }
    memcpy(password_copy, password, pass_len + 1);
    
    int result = PKCS5_PBKDF2_HMAC(
        password_copy,
        pass_len,
        salt,
        sizeof(salt),
        ITERATIONS,
        EVP_sha256(),
        sizeof(hash),
        hash
    );

    OPENSSL_cleanse(password_copy, pass_len + 1);
    free(password_copy);
    password_copy = NULL;

    if (result != 1) {
        fprintf(stderr, "Error in PKCS5_PBKDF2_HMAC.\n");
        ERR_print_errors_fp(stderr);
        return -1;
    }

    char salt_hex[SALT_SIZE * 2 + 1];
    char hash_hex[KEY_LENGTH * 2 + 1];
    bytes_to_hex(salt, sizeof(salt), salt_hex, sizeof(salt_hex));
    bytes_to_hex(hash, sizeof(hash), hash_hex, sizeof(hash_hex));
    
    snprintf(output_buffer, output_buffer_size, "%s:%s", salt_hex, hash_hex);

    return 0;
}

void insert_into_database(const char* username, const char* stored_password) {
    printf("[DB INSERT] Storing credentials for user: %s\n", username);
    printf("  - Username: %s\n", username);
    printf("  - Stored Password (Salt:Hash): %s\n", stored_password);
    puts("--------------------------------------------------");
}

int main() {
    struct User {
        const char* username;
        const char* password;
    };

    struct User test_users[] = {
        {"admin", "AdminPassword123!"},
        {"user01", "s#cureP@ssw0rd"},
        {"test_user", "password"},
        {"another_user", "12345678"},
        {"dave.smith", "P@55w0rdIsDave"}
    };
    int num_users = sizeof(test_users) / sizeof(test_users[0]);
    
    char stored_password_buffer[200]; 

    for (int i = 0; i < num_users; ++i) {
        printf("Processing user: %s\n", test_users[i].username);
        
        memset(stored_password_buffer, 0, sizeof(stored_password_buffer));

        if (register_user(test_users[i].username, test_users[i].password, stored_password_buffer, sizeof(stored_password_buffer)) == 0) {
            insert_into_database(test_users[i].username, stored_password_buffer);
        } else {
            fprintf(stderr, "Failed to hash password for user %s\n", test_users[i].username);
        }
    }

    return 0;
}