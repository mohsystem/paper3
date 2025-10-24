#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

// Compile with: gcc your_file.c -o your_app -lssl -lcrypto

#define ITERATIONS 210000
#define KEY_LENGTH 32
#define SALT_LENGTH 16

// Caller must free the returned string.
char* bytes_to_hex(const unsigned char* bytes, size_t len) {
    char* hex_str = (char*)malloc(len * 2 + 1);
    if (!hex_str) return NULL;
    for (size_t i = 0; i < len; i++) {
        sprintf(hex_str + i * 2, "%02x", bytes[i]);
    }
    hex_str[len * 2] = '\0';
    return hex_str;
}

// Caller must free the returned buffer.
unsigned char* hex_to_bytes(const char* hex_str, size_t* out_len) {
    size_t hex_len = strlen(hex_str);
    if (hex_len % 2 != 0) return NULL;
    *out_len = hex_len / 2;
    unsigned char* bytes = (unsigned char*)malloc(*out_len);
    if (!bytes) return NULL;
    for (size_t i = 0; i < *out_len; i++) {
        if (sscanf(hex_str + i * 2, "%2hhx", &bytes[i]) != 1) {
            free(bytes);
            return NULL;
        }
    }
    return bytes;
}

int store_password(const char* file_path, const char* password) {
    unsigned char salt[SALT_LENGTH];
    unsigned char hash[KEY_LENGTH];
    int result = -1; // Error by default
    char* salt_hex = NULL;
    char* hash_hex = NULL;
    FILE* fp = NULL;

    if (RAND_bytes(salt, sizeof(salt)) != 1) goto cleanup;
    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, sizeof(salt),
                         ITERATIONS, EVP_sha256(), sizeof(hash), hash) != 1) goto cleanup;

    salt_hex = bytes_to_hex(salt, sizeof(salt));
    hash_hex = bytes_to_hex(hash, sizeof(hash));
    if (!salt_hex || !hash_hex) goto cleanup;

    fp = fopen(file_path, "w");
    if (!fp) goto cleanup;

    if (fprintf(fp, "%s:%s", salt_hex, hash_hex) < 0) goto cleanup;

    result = 0; // Success

cleanup:
    if (fp) fclose(fp);
    free(salt_hex);
    free(hash_hex);
    OPENSSL_cleanse(hash, sizeof(hash));
    return result;
}

int verify_password(const char* file_path, const char* password) {
    int result = 0; // 0 = no match / error, 1 = match
    FILE* fp = NULL;
    char line_buffer[SALT_LENGTH * 2 + KEY_LENGTH * 2 + 2]; // salt_hex + hash_hex + ':' + '\0'
    unsigned char* salt = NULL;
    unsigned char* stored_hash = NULL;
    unsigned char test_hash[KEY_LENGTH];
    
    fp = fopen(file_path, "r");
    if (!fp) goto cleanup;

    if (fgets(line_buffer, sizeof(line_buffer), fp) == NULL) goto cleanup;
    
    char* delimiter = strchr(line_buffer, ':');
    if (!delimiter) goto cleanup;
    *delimiter = '\0';
    char* salt_hex = line_buffer;
    char* hash_hex = delimiter + 1;
    // Trim newline if present
    hash_hex[strcspn(hash_hex, "\r\n")] = 0;

    size_t salt_len, stored_hash_len;
    salt = hex_to_bytes(salt_hex, &salt_len);
    stored_hash = hex_to_bytes(hash_hex, &stored_hash_len);

    if (!salt || !stored_hash || salt_len != SALT_LENGTH || stored_hash_len != KEY_LENGTH) {
        goto cleanup;
    }

    if (PKCS5_PBKDF2_HMAC(password, strlen(password), salt, salt_len,
                         ITERATIONS, EVP_sha256(), sizeof(test_hash), test_hash) != 1) {
        goto cleanup;
    }

    if (CRYPTO_memcmp(stored_hash, test_hash, KEY_LENGTH) == 0) {
        result = 1; // Match
    }

cleanup:
    if (fp) fclose(fp);
    free(salt);
    free(stored_hash);
    OPENSSL_cleanse(test_hash, sizeof(test_hash));
    return result;
}

int main() {
    const char* test_file_path = "password_storage_c.txt";
    const char* correct_password = "PasswordForCApp1$";

    printf("--- C Test Cases for Password Verification ---\n");

    if (store_password(test_file_path, correct_password) != 0) {
        fprintf(stderr, "Failed to set up test file. Aborting tests.\n");
        return 1;
    }
    printf("Test file setup complete.\n");

    // Test Case 1: Correct password
    printf("1. Correct password ('%s'): %s\n", correct_password,
           verify_password(test_file_path, correct_password) == 1 ? "PASSED" : "FAILED");

    // Test Case 2: Incorrect password
    const char* wrong_password = "WrongPassword";
    printf("2. Incorrect password ('%s'): %s\n", wrong_password,
           verify_password(test_file_path, wrong_password) == 0 ? "PASSED" : "FAILED");
           
    // Test Case 3: Empty password
    printf("3. Empty password ('%s'): %s\n", "",
           verify_password(test_file_path, "") == 0 ? "PASSED" : "FAILED");

    // Test Case 4: Password with a small difference
    const char* diff_password = "PasswordForCApp1@";
    printf("4. Slightly different password ('%s'): %s\n", diff_password,
           verify_password(test_file_path, diff_password) == 0 ? "PASSED" : "FAILED");

    // Test Case 5: Non-existent file
    printf("5. Non-existent file: %s\n",
           verify_password("nonexistentfile.txt", correct_password) == 0 ? "PASSED" : "FAILED");

    if (remove(test_file_path) == 0) {
        printf("Test file cleaned up.\n");
    } else {
        perror("Error cleaning up test file");
    }

    return 0;
}