#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
#include <fcntl.h>
#include <unistd.h>
#endif

/**
 * Generates a secure random token encoded in hexadecimal.
 * This function uses OS-specific cryptographically secure random number generators:
 * - Windows: BCryptGenRandom
 * - Linux/macOS: /dev/urandom
 *
 * @param lengthInBytes The number of random bytes to generate.
 * @return A dynamically allocated hex-encoded string. The caller is responsible for freeing
 *         this memory using free(). Returns NULL on failure.
 */
char* generateToken(int lengthInBytes) {
    if (lengthInBytes <= 0) {
        return NULL;
    }
    
    unsigned char* buffer = (unsigned char*)malloc(lengthInBytes);
    if (buffer == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for random bytes.\n");
        return NULL;
    }

#ifdef _WIN32
    NTSTATUS status = BCryptGenRandom(NULL, buffer, lengthInBytes, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptGenRandom failed.\n");
        free(buffer);
        return NULL;
    }
#else
    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd == -1) {
        perror("Error opening /dev/urandom");
        free(buffer);
        return NULL;
    }
    ssize_t bytes_read = read(urandom_fd, buffer, lengthInBytes);
    close(urandom_fd);
    if (bytes_read < lengthInBytes) {
        fprintf(stderr, "Error: Not enough bytes read from /dev/urandom.\n");
        free(buffer);
        return NULL;
    }
#endif

    // Allocate memory for the hex string (2 chars per byte + null terminator)
    char* hex_token = (char*)malloc(lengthInBytes * 2 + 1);
    if (hex_token == NULL) {
        fprintf(stderr, "Error: Failed to allocate memory for hex token.\n");
        free(buffer);
        return NULL;
    }

    for (int i = 0; i < lengthInBytes; i++) {
        sprintf(hex_token + (i * 2), "%02x", buffer[i]);
    }
    hex_token[lengthInBytes * 2] = '\0';

    free(buffer);
    return hex_token;
}

int main() {
    printf("C: Generating 5 secure tokens (32 bytes of randomness each)\n");
    for (int i = 0; i < 5; ++i) {
        char* token = generateToken(32);
        if (token != NULL) {
            printf("Test Case %d: %s\n", i + 1, token);
            free(token); // Important: free the memory allocated by generateToken
        }
    }
    return 0;
}