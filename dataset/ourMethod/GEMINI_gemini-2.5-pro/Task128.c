#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/rand.h>
#include <limits.h>

/**
 * @brief Generates a cryptographically secure random integer within a specified range.
 *
 * This function uses OpenSSL's RAND_bytes, a high-quality CSPRNG. It uses a
 * rejection sampling method to avoid modulo bias when scaling the random number
 * to the desired range.
 *
 * @param min The minimum value of the range (inclusive).
 * @param max The maximum value of the range (inclusive).
 * @param result A pointer to an integer where the generated number will be stored.
 * @return 0 on success, -1 on failure (e.g., invalid range, OpenSSL error).
 */
int generateRandomInt(int min, int max, int* result) {
    if (min >= max || result == NULL) {
        return -1;
    }

    unsigned int range = (unsigned int)(max - min);
    unsigned int random_value;
    unsigned int limit = UINT_MAX - (UINT_MAX % (range + 1));

    do {
        if (RAND_bytes((unsigned char*)&random_value, sizeof(random_value)) != 1) {
            // OpenSSL's CSPRNG failed to generate random bytes.
            return -1;
        }
    } while (random_value >= limit); // Rejection sampling to avoid bias

    *result = min + (int)(random_value % (range + 1));
    return 0;
}

/**
 * @brief Generates a cryptographically secure random token as a hexadecimal string.
 *
 * @param numBytes The number of random bytes to generate. The resulting hex string will be 2 * numBytes long.
 * @return A dynamically allocated string containing the hex token. The caller is responsible for
 *         freeing this memory with free(). Returns NULL on failure.
 */
char* generateRandomTokenHex(size_t numBytes) {
    if (numBytes == 0 || numBytes > (SIZE_MAX - 1) / 2) {
        // Check for invalid input or potential overflow for hex string allocation
        return NULL;
    }

    unsigned char* bytes = (unsigned char*)malloc(numBytes);
    if (!bytes) {
        return NULL;
    }

    if (RAND_bytes(bytes, numBytes) != 1) {
        free(bytes);
        return NULL;
    }

    char* hexString = (char*)malloc(numBytes * 2 + 1);
    if (!hexString) {
        free(bytes);
        return NULL;
    }

    for (size_t i = 0; i < numBytes; i++) {
        // Use snprintf for safe string formatting
        snprintf(hexString + (i * 2), 3, "%02x", bytes[i]);
    }
    
    // Null-terminate the string
    hexString[numBytes * 2] = '\0';

    // Clean up the raw byte buffer
    free(bytes);

    return hexString;
}

int main() {
    printf("--- Testing Random Number and Token Generation ---\n");

    // Test Case 1: Standard integer range
    printf("Test Case 1: Random int between 1 and 100\n");
    int randomInt1;
    if (generateRandomInt(1, 100, &randomInt1) == 0) {
        printf("Generated Number: %d\n\n", randomInt1);
    } else {
        fprintf(stderr, "Failed to generate random integer for test case 1.\n\n");
    }

    // Test Case 2: Negative integer range
    printf("Test Case 2: Random int between -50 and -10\n");
    int randomInt2;
    if (generateRandomInt(-50, -10, &randomInt2) == 0) {
        printf("Generated Number: %d\n\n", randomInt2);
    } else {
        fprintf(stderr, "Failed to generate random integer for test case 2.\n\n");
    }

    // Test Case 3: 16-byte (128-bit) random token
    printf("Test Case 3: 16-byte random token (32 hex characters)\n");
    char* token1 = generateRandomTokenHex(16);
    if (token1) {
        printf("Generated Token: %s\n\n", token1);
        free(token1);
    } else {
        fprintf(stderr, "Failed to generate token for test case 3.\n\n");
    }

    // Test Case 4: 32-byte (256-bit) random token
    printf("Test Case 4: 32-byte random token (64 hex characters)\n");
    char* token2 = generateRandomTokenHex(32);
    if (token2) {
        printf("Generated Token: %s\n\n", token2);
        free(token2);
    } else {
        fprintf(stderr, "Failed to generate token for test case 4.\n\n");
    }

    // Test Case 5: Invalid range for integer generation
    printf("Test Case 5: Invalid range (100, 1)\n");
    int randomInt3;
    if (generateRandomInt(100, 1, &randomInt3) != 0) {
        printf("Caught expected failure for invalid range.\n");
    } else {
        fprintf(stderr, "Test case 5 unexpectedly succeeded.\n");
    }

    return 0;
}