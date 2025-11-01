#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>

/**
 * @brief Generates a cryptographically secure random number greater than 1000.
 *
 * NOTE: This implementation is for POSIX-compliant systems (like Linux, macOS)
 * that provide /dev/urandom. It is not portable to all systems (e.g., Windows).
 * On Windows, the Cryptography API (CryptGenRandom) should be used.
 *
 * @return A random integer greater than 1000, or -1 on failure.
 */
int generateRandomNumberGreaterThan1000() {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("Error: Failed to open /dev/urandom");
        return -1;
    }

    unsigned int random_value;
    ssize_t bytes_read = read(fd, &random_value, sizeof(random_value));
    
    // It's good practice to close the file descriptor as soon as it's no longer needed.
    if (close(fd) == -1) {
        perror("Warning: Failed to close /dev/urandom");
    }

    if (bytes_read < 0) {
        perror("Error: Failed to read from /dev/urandom");
        return -1;
    }
    
    if (bytes_read != sizeof(random_value)) {
        fprintf(stderr, "Error: Incomplete read from /dev/urandom\n");
        return -1;
    }

    // Map the random value to the desired range [1001, INT_MAX].
    // The size of the target range.
    long long range_size = (long long)INT_MAX - 1001 + 1;
    
    // Using modulo on a large range like this results in negligible bias.
    // A more rigorous method would use rejection sampling to eliminate bias completely.
    return (random_value % range_size) + 1001;
}

int main() {
    printf("Generating 5 random numbers greater than 1000:\n");
    for (int i = 0; i < 5; ++i) {
        int random_num = generateRandomNumberGreaterThan1000();
        if (random_num != -1) {
            printf("Test Case %d: %d\n", i + 1, random_num);
        } else {
            // If generation failed, stop the test cases.
            fprintf(stderr, "Could not generate random number for Test Case %d.\n", i + 1);
            return 1; // Indicate failure
        }
    }
    return 0;
}