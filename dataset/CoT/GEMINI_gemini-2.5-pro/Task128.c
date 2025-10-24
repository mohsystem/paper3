#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// Note: This implementation is for POSIX-compliant systems (Linux, macOS, etc.)
// that provide /dev/urandom. For Windows, you would use the Cryptography API,
// such as the BCryptGenRandom function. The standard C rand() function is
// not cryptographically secure and should not be used for tokens or secrets.

#if defined(__unix__) || defined(__APPLE__)
#include <fcntl.h>
#include <unistd.h>

/**
 * Generates a cryptographically secure random integer within a specified range.
 * @param min The minimum value (inclusive).
 * @param max The maximum value (inclusive).
 * @return A random integer between min and max, or exits on failure.
 */
int generateRandomInt(int min, int max) {
    if (min >= max) {
        fprintf(stderr, "Error: max must be greater than min.\n");
        return min; // Return a predictable value on error
    }

    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd < 0) {
        perror("Failed to open /dev/urandom");
        exit(EXIT_FAILURE);
    }

    unsigned int range = (unsigned int)(max - min) + 1;
    unsigned int random_value;
    // Use rejection sampling to avoid modulo bias
    unsigned int threshold = UINT_MAX - (UINT_MAX % range);

    do {
        ssize_t result = read(urandom_fd, &random_value, sizeof(random_value));
        if (result < (ssize_t)sizeof(random_value)) {
             perror("Failed to read from /dev/urandom");
             close(urandom_fd);
             exit(EXIT_FAILURE);
        }
    } while (random_value >= threshold);

    close(urandom_fd);
    return min + (random_value % range);
}

/**
 * Generates a cryptographically secure random token.
 * The caller must provide a buffer of at least (length + 1) bytes.
 * @param buffer The buffer to write the null-terminated token into.
 * @param length The desired length of the token (excluding the null terminator).
 */
void generateRandomToken(char *buffer, int length) {
    if (buffer == NULL || length <= 0) {
        if (buffer != NULL) buffer[0] = '\0';
        return;
    }
    
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const int charset_size = sizeof(charset) - 1;

    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd < 0) {
        perror("Failed to open /dev/urandom");
        exit(EXIT_FAILURE);
    }
    
    // Use rejection sampling to avoid modulo bias
    unsigned char random_byte;
    unsigned char threshold = 255 - (255 % charset_size);

    for (int i = 0; i < length; ) {
        ssize_t result = read(urandom_fd, &random_byte, sizeof(random_byte));
        if (result < (ssize_t)sizeof(random_byte)) {
             perror("Failed to read from /dev/urandom");
             close(urandom_fd);
             exit(EXIT_FAILURE);
        }
        if (random_byte < threshold) {
            buffer[i++] = charset[random_byte % charset_size];
        }
    }
    buffer[length] = '\0';
    close(urandom_fd);
}

#else
// Provide stub functions for non-POSIX systems to allow compilation.
// These are NOT secure and just print a warning.
int generateRandomInt(int min, int max) {
    fprintf(stderr, "Warning: Secure random number generation is not implemented for this OS.\n");
    return min;
}

void generateRandomToken(char *buffer, int length) {
    fprintf(stderr, "Warning: Secure token generation is not implemented for this OS.\n");
    if (buffer != NULL && length > 0) {
        strncpy(buffer, "unsupported", length);
        buffer[length] = '\0';
    }
}
#endif

int main() {
    printf("C Random Generation Test Cases:\n");
    
    // Test Case 1
    printf("1. Random integer between 1 and 100: %d\n", generateRandomInt(1, 100));
    
    // Test Case 2
    printf("2. Random integer between -50 and 50: %d\n", generateRandomInt(-50, 50));
    
    // Test Case 3
    char token8[9]; // 8 chars + null terminator
    generateRandomToken(token8, 8);
    printf("3. Random token of length 8: %s\n", token8);
    
    // Test Case 4
    char token16[17]; // 16 chars + null terminator
    generateRandomToken(token16, 16);
    printf("4. Random token of length 16: %s\n", token16);
    
    // Test Case 5
    printf("5. Random integer between 1000 and 5000: %d\n", generateRandomInt(1000, 5000));
    
    return 0;
}