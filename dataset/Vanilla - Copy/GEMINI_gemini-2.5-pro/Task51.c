#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/**
 * Encrypts a string using the Caesar cipher algorithm.
 * @param text The string to encrypt.
 * @param shift The number of positions to shift letters.
 * @return A new dynamically allocated string with the encrypted text. The caller is responsible for freeing this memory.
 */
char* encrypt(const char* text, int shift) {
    if (text == NULL) {
        return NULL;
    }

    int len = strlen(text);
    char* result = (char*)malloc(sizeof(char) * (len + 1));
    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return NULL;
    }

    for (int i = 0; i < len; i++) {
        if (text[i] >= 'a' && text[i] <= 'z') {
            result[i] = ((text[i] - 'a' + shift) % 26) + 'a';
        } else if (text[i] >= 'A' && text[i] <= 'Z') {
            result[i] = ((text[i] - 'A' + shift) % 26) + 'A';
        } else {
            result[i] = text[i];
        }
    }
    result[len] = '\0'; // Null-terminate the string

    return result;
}

void run_test_case(const char* test_name, const char* text, int shift) {
    printf("%s:\n", test_name);
    printf("Original:  %s\n", text);
    printf("Shift:     %d\n", shift);
    char* encrypted_text = encrypt(text, shift);
    if (encrypted_text) {
        printf("Encrypted: %s\n", encrypted_text);
        free(encrypted_text); // Free the memory allocated by encrypt function
    }
    printf("--------------------\n");
}

int main() {
    // Test Case 1
    run_test_case("Test Case 1", "Hello, World!", 3);

    // Test Case 2
    run_test_case("Test Case 2", "Programming is fun!", 7);

    // Test Case 3
    run_test_case("Test Case 3", "Caesar Cipher", 13); // ROT13

    // Test Case 4
    run_test_case("Test Case 4", "All your base are belong to us.", 1);

    // Test Case 5
    run_test_case("Test Case 5", "Testing with numbers 123 and symbols !@#.", 5);

    return 0;
}