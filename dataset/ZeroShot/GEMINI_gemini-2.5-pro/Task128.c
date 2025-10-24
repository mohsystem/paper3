#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// This implementation uses system-specific APIs for cryptographic randomness.
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib") // For MSVC linker

// Helper to get random bytes on Windows using the CNG API
static int get_secure_random_bytes(unsigned char* buf, size_t len) {
    NTSTATUS status = BCryptGenRandom(NULL, buf, (ULONG)len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status < 0) { // Check for failure (NT_SUCCESS(status) is >= 0)
        return -1;
    }
    return 0;
}

#else
#include <fcntl.h>
#include <unistd.h>

// Helper to get random bytes on POSIX systems by reading /dev/urandom
static int get_secure_random_bytes(unsigned char* buf, size_t len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open /dev/urandom");
        return -1;
    }
    ssize_t bytes_read = read(fd, buf, len);
    close(fd);
    if (bytes_read < (ssize_t)len) {
        fprintf(stderr, "Failed to read enough bytes from /dev/urandom\n");
        return -1;
    }
    return 0;
}
#endif

/**
 * Generates a cryptographically secure random integer in [min, max].
 * Uses rejection sampling to avoid modulo bias.
 *
 * @param min The minimum value of the range (inclusive).
 * @param max The maximum value of the range (inclusive).
 * @param error Pointer to an integer for error state (0=success, -1=failure).
 * @return A secure random integer, or 0 on failure.
 */
int generateSecureRandomNumber(int min, int max, int* error) {
    *error = 0;
    if (min > max) {
        fprintf(stderr, "Error: Max must be greater than or equal to Min.\n");
        *error = -1;
        return 0;
    }

    unsigned int range = (unsigned int)(max - min) + 1;
    // If range is 0, it means max-min wrapped around, covering all uint values.
    // In this case, just return the raw random value.
    if (range == 0) {
        unsigned int result;
        if (get_secure_random_bytes((unsigned char*)&result, sizeof(result)) != 0) {
            *error = -1;
            return 0;
        }
        return (int)result;
    }

    unsigned int random_val;
    unsigned int cutoff = (UINT_MAX / range) * range;

    do {
        if (get_secure_random_bytes((unsigned char*)&random_val, sizeof(random_val)) != 0) {
            *error = -1;
            return 0;
        }
    } while (random_val >= cutoff);

    return min + (random_val % range);
}

/**
 * Generates a cryptographically secure random alphanumeric token.
 * The caller is responsible for freeing the returned memory with free().
 *
 * @param length The desired length of the token.
 * @return A dynamically allocated string with the token, or NULL on failure.
 */
char* generateSecureToken(size_t length) {
    if (length == 0) {
        return NULL;
    }
    
    char* token = (char*)malloc(length + 1);
    if (token == NULL) {
        perror("Failed to allocate memory for token");
        return NULL;
    }
    
    const char token_chars[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789";
    const int num_token_chars = sizeof(token_chars) - 1;
    const int cutoff = (256 / num_token_chars) * num_token_chars;

    for (size_t i = 0; i < length; ++i) {
        unsigned char random_byte;
        do {
            if (get_secure_random_bytes(&random_byte, 1) != 0) {
                free(token);
                return NULL;
            }
        } while (random_byte >= cutoff);
        
        token[i] = token_chars[random_byte % num_token_chars];
    }
    token[length] = '\0';
    
    return token;
}


int main() {
    printf("--- Secure Random Number Generation ---\n");
    for (int i = 0; i < 5; ++i) {
        int min = i * 10;
        int max = 50 + i * 10;
        int error;
        int randomNumber = generateSecureRandomNumber(min, max, &error);
        if (error == 0) {
            printf("Test %d: Random number between %d and %d: %d\n", i + 1, min, max, randomNumber);
        } else {
            printf("Test %d: Failed to generate random number.\n", i + 1);
        }
    }

    printf("\n--- Secure Random Token Generation ---\n");
    for (int i = 0; i < 5; ++i) {
        size_t length = 8 + i * 4;
        char* token = generateSecureToken(length);
        if (token != NULL) {
            printf("Test %d: Token of length %zu: %s\n", i + 1, length, token);
            free(token); // Free the allocated memory
        } else {
            printf("Test %d: Failed to generate token of length %zu.\n", i + 1, length);
        }
    }

    return 0;
}