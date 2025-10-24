#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Generates a cryptographically secure random string of a given length.
 *
 * The string will consist only of ASCII letters (a-z, A-Z).
 * This implementation reads from /dev/urandom and is suitable for POSIX-like
 * systems (Linux, macOS). On Windows, an alternative like BCryptGenRandom
 * from the Crypto API should be used.
 * The caller is responsible for freeing the returned string.
 *
 * @param length The desired length of the string.
 * @return A dynamically allocated random string of the specified length,
 *         or an empty string if length is <= 0. Returns NULL on failure.
 */
char* generateRandomString(int length) {
    if (length <= 0) {
        char* empty_str = (char*)malloc(1);
        if (empty_str) {
            empty_str[0] = '\0';
        }
        return empty_str;
    }

    static const char ALPHABET[] =
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const int alphabet_size = sizeof(ALPHABET) - 1;

    char* result = (char*)malloc(length + 1);
    if (!result) {
        perror("malloc failed");
        return NULL;
    }

    FILE* urandom = fopen("/dev/urandom", "rb");
    if (!urandom) {
        perror("Failed to open /dev/urandom");
        free(result);
        return NULL;
    }

    // Use rejection sampling to avoid modulo bias.
    // The largest multiple of alphabet_size (52) that is <= 256 is 208 (52 * 4).
    // We will discard any random byte >= 208.
    const int usable_range_limit = (256 / alphabet_size) * alphabet_size;

    for (int i = 0; i < length; ) {
        unsigned char random_byte;
        if (fread(&random_byte, 1, 1, urandom) != 1) {
            fprintf(stderr, "Failed to read from /dev/urandom\n");
            fclose(urandom);
            free(result);
            return NULL;
        }
        
        if (random_byte < usable_range_limit) {
            result[i] = ALPHABET[random_byte % alphabet_size];
            i++;
        }
    }

    result[length] = '\0';
    fclose(urandom);

    return result;
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Length 10
    char* s1 = generateRandomString(10);
    if (s1) {
        printf("Test 1 (length 10): %s (length: %zu)\n", s1, strlen(s1));
        free(s1);
    }

    // Test Case 2: Length 0
    char* s2 = generateRandomString(0);
    if (s2) {
        printf("Test 2 (length 0): \"%s\" (length: %zu)\n", s2, strlen(s2));
        free(s2);
    }

    // Test Case 3: Length 1
    char* s3 = generateRandomString(1);
    if (s3) {
        printf("Test 3 (length 1): %s (length: %zu)\n", s3, strlen(s3));
        free(s3);
    }

    // Test Case 4: Length 32
    char* s4 = generateRandomString(32);
    if (s4) {
        printf("Test 4 (length 32): %s (length: %zu)\n", s4, strlen(s4));
        free(s4);
    }

    // Test Case 5: Negative length
    char* s5 = generateRandomString(-5);
    if (s5) {
        printf("Test 5 (length -5): \"%s\" (length: %zu)\n", s5, strlen(s5));
        free(s5);
    }
    
    return 0;
}