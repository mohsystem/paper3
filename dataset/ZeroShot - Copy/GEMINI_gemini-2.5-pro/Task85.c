#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * NOTE: This C implementation is for POSIX-compliant systems (like Linux, macOS)
 * that provide /dev/urandom as a source of cryptographically secure random numbers.
 * For Windows, the CryptoAPI (e.g., CryptGenRandom) or CNG (BCryptGenRandom)
 * should be used instead.
 */
#if defined(__unix__) || defined(__APPLE__)
#include <fcntl.h>
#include <unistd.h>
#endif

/**
 * @brief Generates a cryptographically secure random string of a specified length.
 * The string consists only of ASCII letters (a-z, A-Z).
 * The caller is responsible for freeing the returned string using free().
 * 
 * @param length The desired length of the string. Must be non-negative.
 * @return A dynamically allocated random string, or NULL on failure.
 *         Returns an empty, dynamically allocated string if length is <= 0.
 */
char* generateRandomString(int length) {
    if (length <= 0) {
        char* emptyStr = (char*)malloc(1);
        if (emptyStr == NULL) return NULL;
        emptyStr[0] = '\0';
        return emptyStr;
    }

    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    const int charset_size = sizeof(charset) - 1; // Exclude null terminator

    char* random_string = (char*)malloc(length + 1);
    if (random_string == NULL) {
        perror("Unable to allocate memory for the string");
        return NULL;
    }

#if defined(__unix__) || defined(__APPLE__)
    int random_data_fd = open("/dev/urandom", O_RDONLY);
    if (random_data_fd == -1) {
        perror("Error opening /dev/urandom");
        free(random_string);
        return NULL;
    }

    unsigned char* random_bytes = (unsigned char*)malloc(length);
    if (random_bytes == NULL) {
        perror("Unable to allocate memory for random bytes");
        free(random_string);
        close(random_data_fd);
        return NULL;
    }

    ssize_t bytes_read = read(random_data_fd, random_bytes, length);
    if (bytes_read < length) {
        fprintf(stderr, "Error: Could not read enough bytes from /dev/urandom\n");
        close(random_data_fd);
        free(random_string);
        free(random_bytes);
        return NULL;
    }
    close(random_data_fd);

    for (int i = 0; i < length; i++) {
        // Use modulo to map the random byte to an index in the charset.
        random_string[i] = charset[random_bytes[i] % charset_size];
    }
    free(random_bytes);

#else
    // Fallback for non-POSIX systems (e.g., Windows) - NOT SECURE
    // A proper implementation would use Windows CryptoAPI.
    // This is just a placeholder to allow compilation.
    srand((unsigned int)time(NULL)); // Insecure, for demonstration only
    for (int i = 0; i < length; ++i) {
        random_string[i] = charset[rand() % charset_size];
    }
    #warning "Using insecure rand() because /dev/urandom is not available. This is not for production use."
#endif

    random_string[length] = '\0';
    return random_string;
}

int main() {
    printf("C Test Cases:\n");
    int testLengths[] = {16, 32, 1, 0, -5};
    int num_tests = sizeof(testLengths) / sizeof(testLengths[0]);

    for (int i = 0; i < num_tests; i++) {
        int len = testLengths[i];
        char* randomStr = generateRandomString(len);
        if (randomStr != NULL) {
            printf("Length %d -> \"%s\" (Actual length: %zu)\n", len, randomStr, strlen(randomStr));
            free(randomStr); // Free the allocated memory
        } else {
            printf("Length %d -> Generation failed.\n", len);
        }
    }
    return 0;
}