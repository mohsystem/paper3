#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * @brief Encrypts a message using the Vigenère cipher.
 * 
 * @param message The plaintext message to encrypt.
 * @param key The secret key for encryption.
 * @return A new dynamically allocated string containing the ciphertext. 
 *         The caller is responsible for freeing this memory.
 */
char* encrypt(const char* message, const char* key) {
    size_t key_len = strlen(key);
    if (key_len == 0) {
        char* result = strdup(message);
        return result;
    }
    
    size_t message_len = strlen(message);
    char* encrypted_message = (char*)malloc(message_len + 1);
    if (encrypted_message == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    char* upper_key = (char*)malloc(key_len + 1);
     if (upper_key == NULL) {
        perror("Failed to allocate memory");
        free(encrypted_message);
        return NULL;
    }

    for(size_t i = 0; i < key_len; i++) {
        upper_key[i] = toupper(key[i]);
    }
    upper_key[key_len] = '\0';

    int key_index = 0;
    for (size_t i = 0; i < message_len; i++) {
        if (isalpha(message[i])) {
            char key_char = upper_key[key_index % key_len];
            int shift = key_char - 'A';
            encrypted_message[i] = ((toupper(message[i]) - 'A' + shift) % 26) + 'A';
            key_index++;
        } else {
            encrypted_message[i] = message[i]; // Preserve original non-letter characters
        }
    }
    encrypted_message[message_len] = '\0';
    
    free(upper_key);
    return encrypted_message;
}

int main() {
    const char* messages[] = {
        "ATTACK AT DAWN",
        "HELLO WORLD",
        "Cryptography",
        "This is a test message 123!",
        "ALLCAPS"
    };
    const char* keys[] = {
        "LEMON",
        "KEY",
        "SECRET",
        "CIPHER",
        "ALLCAPS"
    };
    int num_tests = sizeof(messages) / sizeof(messages[0]);

    for (int i = 0; i < num_tests; i++) {
        char* encrypted = encrypt(messages[i], keys[i]);
        
        printf("Test Case %d:\n", i + 1);
        printf("  Original:  %s\n", messages[i]);
        printf("  Key:       %s\n", keys[i]);
        if (encrypted) {
            printf("  Encrypted: %s\n", encrypted);
            free(encrypted); // Free the dynamically allocated memory
        }
        printf("\n");
    }

    return 0;
}