#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")
#else
// No special includes needed for POSIX /dev/urandom access
#endif

/**
 * Generates a cryptographically secure token as a hex string.
 * The caller is responsible for freeing the returned string using free().
 *
 * @param byte_length The number of random bytes to generate. 32 is recommended.
 * @return A dynamically allocated hexadecimal string, or NULL on failure.
 */
char* generate_token(size_t byte_length) {
    if (byte_length == 0) {
        return NULL;
    }

    // Allocate buffer for random bytes
    unsigned char* random_bytes = (unsigned char*)malloc(byte_length);
    if (!random_bytes) {
        perror("Failed to allocate memory for random bytes");
        return NULL;
    }

    // Generate random bytes using OS-specific CSPRNG
#ifdef _WIN32
    BCRYPT_ALG_HANDLE hAlg = NULL;
    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_RNG_ALGORITHM, NULL, 0) != 0) {
        fprintf(stderr, "Failed to open algorithm provider.\n");
        free(random_bytes);
        return NULL;
    }
    if (BCryptGenRandom(hAlg, random_bytes, (ULONG)byte_length, 0) != 0) {
        fprintf(stderr, "Failed to generate random data.\n");
        BCryptCloseAlgorithmProvider(hAlg, 0);
        free(random_bytes);
        return NULL;
    }
    BCryptCloseAlgorithmProvider(hAlg, 0);
#else
    FILE* urandom = fopen("/dev/urandom", "rb");
    if (!urandom) {
        perror("Failed to open /dev/urandom");
        free(random_bytes);
        return NULL;
    }
    if (fread(random_bytes, 1, byte_length, urandom) != byte_length) {
        fprintf(stderr, "Failed to read from /dev/urandom.\n");
        fclose(urandom);
        free(random_bytes);
        return NULL;
    }
    fclose(urandom);
#endif

    // Allocate memory for the hex string: 2 chars per byte + 1 for null terminator
    size_t hex_length = byte_length * 2;
    char* hex_string = (char*)malloc(hex_length + 1);
    if (!hex_string) {
        perror("Failed to allocate memory for hex string");
        free(random_bytes);
        return NULL;
    }

    // Convert bytes to hex string
    for (size_t i = 0; i < byte_length; ++i) {
        sprintf(hex_string + (i * 2), "%02x", random_bytes[i]);
    }
    hex_string[hex_length] = '\0';

    free(random_bytes);
    return hex_string;
}

int main() {
    printf("C Token Generation Test Cases:\n");
    char* token = NULL;

    // Test Case 1: 16 bytes (128 bits) -> 32 hex chars
    token = generate_token(16);
    if (token) {
        printf("1. 16-byte token: %s\n", token);
        free(token);
    }

    // Test Case 2: 32 bytes (256 bits) -> 64 hex chars - Recommended
    token = generate_token(32);
    if (token) {
        printf("2. 32-byte token: %s\n", token);
        free(token);
    }
    
    // Test Case 3: 48 bytes (384 bits) -> 96 hex chars
    token = generate_token(48);
    if (token) {
        printf("3. 48-byte token: %s\n", token);
        free(token);
    }
    
    // Test Case 4: Another 32-byte token to show it's different
    token = generate_token(32);
    if (token) {
        printf("4. 32-byte token: %s\n", token);
        free(token);
    }
    
    // Test Case 5: 64 bytes (512 bits) -> 128 hex chars
    token = generate_token(64);
    if (token) {
        printf("5. 64-byte token: %s\n", token);
        free(token);
    }

    return 0;
}