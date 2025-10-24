#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Platform-specific include for process ID
#ifdef _WIN32
#include <process.h>
#define getpid _getpid
#else
#include <unistd.h>
#endif

/**
 * Generates a random session ID and fills the provided buffer.
 * @param buffer The character buffer to store the session ID.
 * @param buffer_size The total size of the buffer (including null terminator).
 */
void generateSessionId(char* buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size == 0) {
        return;
    }
    
    const char* chars = "abcdef0123456789";
    int num_chars = 16; // strlen(chars)

    for (size_t i = 0; i < buffer_size - 1; ++i) {
        buffer[i] = chars[rand() % num_chars];
    }
    buffer[buffer_size - 1] = '\0';
}

int main() {
    // Seed the random number generator only once at the start of the program.
    // Using time and process ID for better randomness.
    srand((unsigned int)time(NULL) ^ getpid());

    printf("C Session IDs:\n");
    for (int i = 0; i < 5; ++i) {
        char sessionId[33]; // 32 characters + null terminator
        generateSessionId(sessionId, sizeof(sessionId));
        printf("Test Case %d: %s\n", i + 1, sessionId);
    }

    return 0;
}