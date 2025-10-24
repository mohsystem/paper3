#include <stdio.h>
#include <string.h>
#include <stdlib.h>
// Requires OpenSSL library. Compile with: gcc <filename>.c -o <output_name> -l crypto
#include <openssl/evp.h>

#define ITERATIONS 65536
#define KEY_LENGTH 32 // bytes, equivalent to 256 bits

/**
 * Computes the hash of a password combined with a salt using PBKDF2-HMAC-SHA256.
 *
 * @param password   The null-terminated password string to hash.
 * @param salt       A pointer to the salt data.
 * @param salt_len   The length of the salt in bytes.
 * @param hash_out   A buffer to store the resulting hash. Must be at least KEY_LENGTH bytes.
 * @return 1 on success, 0 on failure.
 */
int hash_password(const char* password, 
                  const unsigned char* salt, size_t salt_len,
                  unsigned char* hash_out) {
    if (password == NULL || salt == NULL || hash_out == NULL) {
        return 0; // Indicate failure for null pointers
    }

    int result = PKCS5_PBKDF2_HMAC(
        password,
        (int)strlen(password),
        salt,
        (int)salt_len,
        ITERATIONS,
        EVP_sha256(),
        KEY_LENGTH,
        hash_out
    );

    return result; // 1 for success, 0 for failure
}

/**
 * Helper function to print a block of data as a hexadecimal string.
 */
void print_hex(const char* label, const unsigned char* data, size_t len) {
    printf("%s", label);
    for (size_t i = 0; i < len; ++i) {
        printf("%02x", data[i]);
    }
    printf("\n");
}

int main() {
    // Test Case 1
    const char* password_1 = "password123";
    const unsigned char salt_1[] = "salt1234";
    size_t salt_len_1 = sizeof(salt_1) - 1; // Exclude null terminator

    // Test Case 2
    const char* password_2 = "P@$$w0rd!";
    const unsigned char salt_2[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
    size_t salt_len_2 = sizeof(salt_2);

    // Test Case 3
    const char* password_3 = "correct horse battery staple";
    const unsigned char salt_3[] = "another-random-salt";
    size_t salt_len_3 = sizeof(salt_3) - 1;

    // Test Case 4
    const char* password_4 = "123";
    const unsigned char* salt_4 = (const unsigned char*)""; // Empty salt
    size_t salt_len_4 = 0;

    // Test Case 5
    const char* password_5 = "";
    const unsigned char salt_5[] = "a_very_long_salt_for_testing_purposes_with_an_empty_password";
    size_t salt_len_5 = sizeof(salt_5) - 1;

    // Arrays of tests for easier looping
    const char* passwords[] = {password_1, password_2, password_3, password_4, password_5};
    const unsigned char* salts[] = {salt_1, salt_2, salt_3, salt_4, salt_5};
    size_t salt_lens[] = {salt_len_1, salt_len_2, salt_len_3, salt_len_4, salt_len_5};
    
    unsigned char hash_output[KEY_LENGTH];
    
    printf("C Password Hashing Tests:\n");

    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        printf("Password: \"%s\"\n", passwords[i]);
        print_hex("Salt (hex): ", salts[i], salt_lens[i]);

        if (hash_password(passwords[i], salts[i], salt_lens[i], hash_output)) {
            print_hex("Hashed Password (hex): ", hash_output, KEY_LENGTH);
        } else {
            fprintf(stderr, "Error: Password hashing failed for test case %d.\n", i + 1);
        }
    }

    return 0;
}