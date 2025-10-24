#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
 * @brief Encrypts a string using a repeating key XOR cipher and returns a new hex-encoded string.
 * This is a simple educational cipher and is NOT secure for real-world use.
 * The caller is responsible for freeing the memory of the returned string.
 *
 * @param plaintext The string to encrypt.
 * @param key The encryption key.
 * @return A new dynamically allocated hex-encoded string, or a copy of the plaintext if the key is NULL/empty.
 *         Returns NULL on memory allocation failure.
 */
char* encrypt(const char* plaintext, const char* key) {
    if (plaintext == NULL) return NULL;
    if (key == NULL || key[0] == '\0') {
        char* original_copy = malloc(strlen(plaintext) + 1);
        if (original_copy) {
            strcpy(original_copy, plaintext);
        }
        return original_copy;
    }

    size_t plaintext_len = strlen(plaintext);
    size_t key_len = strlen(key);
    
    // Each character becomes two hex digits, plus one for the null terminator.
    char* ciphertext = (char*)malloc(plaintext_len * 2 + 1);
    if (ciphertext == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    for (size_t i = 0; i < plaintext_len; ++i) {
        unsigned char encrypted_char = (unsigned char)plaintext[i] ^ (unsigned char)key[i % key_len];
        sprintf(ciphertext + i * 2, "%02x", encrypted_char);
    }
    ciphertext[plaintext_len * 2] = '\0';

    return ciphertext;
}

int main() {
    const char* test_plaintexts[] = {
        "Hello, World!",
        "This is a longer test message.",
        "Short",
        "12345!@#$%",
        "Test with empty key"
    };
    const char* test_keys[] = {
        "KEY",
        "SECRET",
        "LONGERKEY",
        "NUMSYM",
        ""
    };
    int num_test_cases = 5;

    printf("C Encryption Test Cases:\n");
    for (int i = 0; i < num_test_cases; ++i) {
        const char* plaintext = test_plaintexts[i];
        const char* key = test_keys[i];
        char* encrypted = encrypt(plaintext, key);

        printf("Plaintext : \"%s\", Key: \"%s\"\n", plaintext, key);
        if (encrypted != NULL) {
            printf("Encrypted : %s\n", encrypted);
            free(encrypted); // Free the allocated memory
        } else {
            printf("Encrypted : (Error or no change)\n");
        }
        printf("--------------------\n");
    }

    return 0;
}