#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Generates an unpredictable random string of a specified length using only ASCII letters.
 *
 * This function reads from /dev/urandom on POSIX-compliant systems (Linux, macOS, etc.)
 * to get cryptographically secure random bytes.
 * NOTE: This implementation is not portable to Windows and requires modification 
 * (e.g., using the BCryptGenRandom function from the Windows Cryptography API).
 *
 * @param length The desired length of the string.
 * @return A dynamically allocated random string of the specified length. The caller is responsible 
 *         for freeing this memory. Returns NULL on failure or an allocated empty string
 *         if length is <= 0.
 */
char* generateRandomString(int length) {
    if (length <= 0) {
        char *empty_str = (char*)malloc(1);
        if (empty_str) {
            empty_str[0] = '\0';
        }
        return empty_str;
    }

    const char CHARS[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int CHARS_LEN = sizeof(CHARS) - 1; // Exclude null terminator

    char* result_str = (char*)malloc(length + 1);
    if (!result_str) {
        perror("Failed to allocate memory for string");
        return NULL;
    }

    FILE *fp = fopen("/dev/urandom", "rb");
    if (!fp) {
        perror("Failed to open /dev/urandom");
        free(result_str);
        return NULL;
    }

    for (int i = 0; i < length; ++i) {
        unsigned char random_byte;
        if (fread(&random_byte, 1, 1, fp) != 1) {
            fprintf(stderr, "Failed to read from /dev/urandom\n");
            fclose(fp);
            free(result_str);
            return NULL;
        }
        // Map the random byte to an index in our character set.
        // NOTE: Using the modulo operator introduces a very slight bias as 256 is not
        // a perfect multiple of 52. For most purposes, this is acceptable.
        result_str[i] = CHARS[random_byte % CHARS_LEN];
    }

    result_str[length] = '\0';
    fclose(fp);
    return result_str;
}

int main() {
    printf("\nC Test Cases:\n");

    // Test Case 1: Standard length
    int len1 = 16;
    char* randomStr1 = generateRandomString(len1);
    if (randomStr1) {
        printf("1. Length %d: %s (Actual length: %zu)\n", len1, randomStr1, strlen(randomStr1));
        free(randomStr1);
    }

    // Test Case 2: Short length
    int len2 = 5;
    char* randomStr2 = generateRandomString(len2);
    if (randomStr2) {
        printf("2. Length %d: %s (Actual length: %zu)\n", len2, randomStr2, strlen(randomStr2));
        free(randomStr2);
    }
    
    // Test Case 3: Long length
    int len3 = 64;
    char* randomStr3 = generateRandomString(len3);
    if (randomStr3) {
        printf("3. Length %d: %s (Actual length: %zu)\n", len3, randomStr3, strlen(randomStr3));
        free(randomStr3);
    }

    // Test Case 4: Zero length
    int len4 = 0;
    char* randomStr4 = generateRandomString(len4);
    if (randomStr4) {
        printf("4. Length %d: \"%s\" (Actual length: %zu)\n", len4, randomStr4, strlen(randomStr4));
        free(randomStr4);
    }

    // Test Case 5: Negative length
    int len5 = -10;
    char* randomStr5 = generateRandomString(len5);
    if (randomStr5) {
        printf("5. Length %d: \"%s\" (Actual length: %zu)\n", len5, randomStr5, strlen(randomStr5));
        free(randomStr5);
    }

    return 0;
}