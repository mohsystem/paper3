#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * Generates a random alphanumeric token.
 * Note: rand() is not cryptographically secure and is used here for simplicity.
 * For production, use a more secure random number generator (e.g., OpenSSL's RAND_bytes).
 *
 * @param tokenBuffer A pre-allocated character buffer to store the token.
 * @param length The desired length of the token (excluding the null terminator).
 */
void generateToken(char* tokenBuffer, int length) {
    if (tokenBuffer == NULL || length <= 0) {
        return;
    }

    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    int charsetSize = sizeof(charset) - 1; // Exclude the null terminator

    for (int i = 0; i < length; ++i) {
        int key = rand() % charsetSize;
        tokenBuffer[i] = charset[key];
    }
    tokenBuffer[length] = '\0'; // Null-terminate the string
}

int main() {
    // Seed the random number generator once at the beginning of the program
    srand(time(NULL));

    int tokenLength = 32;

    printf("C: Generating 5 authentication tokens.\n");
    for (int i = 0; i < 5; ++i) {
        // Allocate memory for the token string + null terminator
        char* token = (char*)malloc(sizeof(char) * (tokenLength + 1));
        if (token == NULL) {
             fprintf(stderr, "Memory allocation failed\n");
             return 1;
        }

        generateToken(token, tokenLength);
        printf("Test Case %d: %s\n", i + 1, token);

        // Free the allocated memory for the token
        free(token);
    }

    return 0;
}