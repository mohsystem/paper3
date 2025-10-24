#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SESSION_ID_BYTES 16
#define SESSION_ID_STRING_LEN (SESSION_ID_BYTES * 2 + 1)

// Platform-specific implementation for getting cryptographically secure random bytes.
#if defined(_WIN32)
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib") // Link against the bcrypt library for BCryptGenRandom

int get_random_bytes(unsigned char* buf, size_t len) {
    NTSTATUS status = BCryptGenRandom(NULL, buf, len, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (!BCRYPT_SUCCESS(status)) {
        fprintf(stderr, "Error: BCryptGenRandom failed.\n");
        return -1; // Failure
    }
    return 0; // Success
}

#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
#include <fcntl.h>
#include <unistd.h>

int get_random_bytes(unsigned char* buf, size_t len) {
    FILE* urandom_file = fopen("/dev/urandom", "rb");
    if (!urandom_file) {
        perror("Error opening /dev/urandom");
        return -1;
    }
    size_t bytes_read = fread(buf, 1, len, urandom_file);
    fclose(urandom_file);
    if (bytes_read < len) {
        fprintf(stderr, "Error: Could not read enough random bytes from /dev/urandom.\n");
        return -1;
    }
    return 0;
}

#else
#error "Unsupported platform: No secure random number source available."
#endif

/**
 * Generates a cryptographically secure, random and unique session ID.
 * The implementation is platform-specific to use the OS's secure random source.
 *
 * @return A dynamically allocated string containing the session ID. The caller
 *         is responsible for freeing this memory with free(). Returns NULL on failure.
 */
char* generateSessionId() {
    unsigned char random_bytes[SESSION_ID_BYTES];
    
    if (get_random_bytes(random_bytes, SESSION_ID_BYTES) != 0) {
        return NULL; // Failed to get random data
    }

    // Allocate memory for the hex string (2 chars per byte + null terminator).
    char* hex_string = (char*)malloc(SESSION_ID_STRING_LEN);
    if (hex_string == NULL) {
        perror("Error allocating memory for session ID");
        return NULL;
    }
    
    // Convert bytes to hex string
    for (int i = 0; i < SESSION_ID_BYTES; i++) {
        // sprintf is safe here because we've allocated exactly the right amount of memory
        // and the format string is fixed.
        sprintf(hex_string + (i * 2), "%02x", random_bytes[i]);
    }
    hex_string[SESSION_ID_STRING_LEN - 1] = '\0'; // Ensure null termination
    
    return hex_string;
}

int main() {
    printf("C: Generating 5 unique session IDs\n");
    for (int i = 0; i < 5; i++) {
        char* sessionId = generateSessionId();
        if (sessionId != NULL) {
            printf("Session ID %d: %s\n", i + 1, sessionId);
            free(sessionId); // IMPORTANT: Free the allocated memory
        } else {
            printf("Failed to generate session ID %d\n", i + 1);
        }
    }
    return 0;
}