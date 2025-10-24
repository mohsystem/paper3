#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

/**
 * @brief Generates a random integer within a specified range (inclusive).
 * Assumes srand() has been called.
 * @param min The minimum value of the range.
 * @param max The maximum value of the range.
 * @return A random integer between min and max.
 */
int generateRandomNumber(int min, int max) {
    if (min > max) {
        // Simple error handling, could return an error code
        return min; 
    }
    return (rand() % (max - min + 1)) + min;
}

/**
 * @brief Generates a random alphanumeric token of a specified length.
 * Assumes srand() has been called.
 * @param buffer A pre-allocated character array to store the token.
 * @param length The desired length of the token (excluding null terminator).
 */
void generateRandomToken(char* buffer, int length) {
    if (length <= 0 || buffer == NULL) {
        if (buffer != NULL) buffer[0] = '\0';
        return;
    }
    const char tokenChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int numTokenChars = sizeof(tokenChars) - 1; // Exclude null terminator

    for (int i = 0; i < length; ++i) {
        int randomIndex = rand() % numTokenChars;
        buffer[i] = tokenChars[randomIndex];
    }
    buffer[length] = '\0'; // Null-terminate the string
}

int main() {
    // Seed the random number generator once at the start of the program
    srand(time(NULL));

    printf("--- C Random Number and Token Generation ---\n");

    // Test cases for random number generation
    printf("\n--- Random Numbers ---\n");
    printf("1. Random number between 1 and 100: %d\n", generateRandomNumber(1, 100));
    printf("2. Random number between -50 and 50: %d\n", generateRandomNumber(-50, 50));
    printf("3. Random number between 1000 and 2000: %d\n", generateRandomNumber(1000, 2000));
    printf("4. Random number between 0 and 1: %d\n", generateRandomNumber(0, 1));
    printf("5. Random number between 5 and 5: %d\n", generateRandomNumber(5, 5));

    // Test cases for random token generation
    printf("\n--- Random Tokens ---\n");
    char tokenBuffer[33]; // Max length 32 + null terminator

    generateRandomToken(tokenBuffer, 8);
    printf("1. Token of length 8: %s\n", tokenBuffer);

    generateRandomToken(tokenBuffer, 12);
    printf("2. Token of length 12: %s\n", tokenBuffer);

    generateRandomToken(tokenBuffer, 16);
    printf("3. Token of length 16: %s\n", tokenBuffer);

    generateRandomToken(tokenBuffer, 1);
    printf("4. Token of length 1: %s\n", tokenBuffer);

    generateRandomToken(tokenBuffer, 32);
    printf("5. Token of length 32: %s\n", tokenBuffer);

    return 0;
}