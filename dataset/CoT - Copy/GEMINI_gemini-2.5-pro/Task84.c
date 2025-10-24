#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
// Link with Advapi32.lib
#pragma comment(lib, "advapi32.lib")
#else // POSIX-compliant systems (Linux, macOS, etc.)
#include <fcntl.h>
#include <unistd.h>
#endif

/**
 * Generates a cryptographically secure random session ID.
 * This function is cross-platform, using CryptGenRandom on Windows and
 * reading from /dev/urandom on POSIX systems.
 *
 * @param buffer The output buffer to store the session ID hex string.
 * @param buffer_len The size of the output buffer. Must be at least 33 bytes
 *                   (16 random bytes * 2 hex chars + 1 null terminator).
 * @return 0 on success, -1 on failure.
 */
int generateSessionId(char* buffer, size_t buffer_len) {
    if (buffer_len < 33) {
        fprintf(stderr, "Error: Buffer too small for session ID.\n");
        return -1;
    }

    unsigned char random_bytes[16];

#ifdef _WIN32
    HCRYPTPROV hCryptProv;
    if (!CryptAcquireContext(&hCryptProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
        fprintf(stderr, "Error: CryptAcquireContext failed.\n");
        return -1;
    }
    if (!CryptGenRandom(hCryptProv, sizeof(random_bytes), random_bytes)) {
        fprintf(stderr, "Error: CryptGenRandom failed.\n");
        CryptReleaseContext(hCryptProv, 0);
        return -1;
    }
    CryptReleaseContext(hCryptProv, 0);
#else
    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd == -1) {
        perror("Error opening /dev/urandom");
        return -1;
    }
    ssize_t bytes_read = read(urandom_fd, random_bytes, sizeof(random_bytes));
    close(urandom_fd);
    if (bytes_read != sizeof(random_bytes)) {
        fprintf(stderr, "Error: Could not read enough random bytes.\n");
        return -1;
    }
#endif

    // Convert random bytes to a hexadecimal string
    for (size_t i = 0; i < sizeof(random_bytes); ++i) {
        sprintf(buffer + (i * 2), "%02x", random_bytes[i]);
    }
    buffer[32] = '\0'; // Null-terminate the string

    return 0;
}

int main() {
    printf("--- Testing generateSessionId() ---\n");
    for (int i = 0; i < 5; ++i) {
        // Buffer size is 32 hex characters + 1 null terminator
        char sessionId[33];
        if (generateSessionId(sessionId, sizeof(sessionId)) == 0) {
            printf("Test Case %d: %s\n", i + 1, sessionId);
        } else {
            printf("Test Case %d: Failed to generate session ID.\n", i + 1);
        }
    }
    return 0;
}