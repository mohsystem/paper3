#include <stdio.h>
#include <stdlib.h>    // For abs()
#include <fcntl.h>     // For O_RDONLY
#include <unistd.h>    // For read(), close()
#include <limits.h>    // For UINT_MAX
#include <errno.h>     // For errno
#include <string.h>    // For strerror

// A struct to hold the results for cleaner return from the function.
struct RandomResult {
    int num1;
    int num2;
    int difference;
    int error; // 0 for success, non-zero for error
};

/**
 * Generates a cryptographically secure random integer within a specified range [min, max].
 * This implementation reads from /dev/urandom and includes a de-biasing step
 * to ensure uniform distribution. It is suitable for Unix-like systems (Linux, macOS).
 * For Windows, a different implementation using BCryptGenRandom would be required.
 *
 * @param min The minimum value of the range (inclusive).
 * @param max The maximum value of the range (inclusive).
 * @param error_ptr Pointer to an integer to store error codes. 0 on success.
 * @return A random integer in the range, or -1 on error.
 */
static int secure_rand_range(int min, int max, int* error_ptr) {
    *error_ptr = 0;
    if (min > max) {
        *error_ptr = 1; // Invalid argument
        return -1;
    }

    // Use unsigned int for range calculation to avoid signed overflow issues.
    unsigned int range = (unsigned int)(max - min) + 1;
    if (range == 0) { // This happens if max - min + 1 > UINT_MAX
        *error_ptr = 1;
        return -1;
    }
    
    unsigned int random_value;
    // De-biasing: we only accept random values in a range that is a multiple
    // of our desired range to ensure a uniform distribution.
    unsigned int limit = UINT_MAX - (UINT_MAX % range);

    int urandom_fd = open("/dev/urandom", O_RDONLY);
    if (urandom_fd < 0) {
        *error_ptr = 2; // Failed to open /dev/urandom
        return -1;
    }
    
    ssize_t bytes_read;
    do {
        bytes_read = read(urandom_fd, &random_value, sizeof(random_value));
        if (bytes_read < (ssize_t)sizeof(random_value)) {
            close(urandom_fd);
            *error_ptr = 3; // Failed to read from /dev/urandom
            return -1;
        }
    } while (random_value >= limit); // Reject biased values

    close(urandom_fd);
    
    return (random_value % range) + min;
}

/**
 * Generates two random integers, calculates their absolute difference, and returns the results.
 *
 * @param min The minimum value of the range (inclusive).
 * @param max The maximum value of the range (inclusive).
 * @return A RandomResult struct containing the numbers, their difference, and an error code.
 */
struct RandomResult generateRandomsAndDifference(int min, int max) {
    struct RandomResult result = {0, 0, 0, 0};
    int error_code = 0;

    result.num1 = secure_rand_range(min, max, &error_code);
    if (error_code != 0) {
        result.error = error_code;
        return result;
    }

    result.num2 = secure_rand_range(min, max, &error_code);
    if (error_code != 0) {
        result.error = error_code;
        return result;
    }

    result.difference = abs(result.num1 - result.num2);
    
    return result;
}

int main() {
    printf("Running 5 test cases...\n");
    // This implementation is for POSIX-compliant systems (Linux, macOS, etc.).
    const int MIN_VAL = 10;
    const int MAX_VAL = 1000;

    for (int i = 0; i < 5; ++i) {
        struct RandomResult result = generateRandomsAndDifference(MIN_VAL, MAX_VAL);
        if (result.error != 0) {
            fprintf(stderr, "Test Case %d: Failed to generate random numbers. ", i + 1);
            if (result.error == 1) fprintf(stderr, "Invalid range provided.\n");
            else if (result.error == 2) fprintf(stderr, "Could not open /dev/urandom: %s\n", strerror(errno));
            else if (result.error == 3) fprintf(stderr, "Could not read from /dev/urandom: %s\n", strerror(errno));
            else fprintf(stderr, "Unknown error code: %d\n", result.error);
        } else {
            printf("Test Case %d: Num1: %d, Num2: %d, Difference: %d\n",
                   i + 1, result.num1, result.num2, result.difference);
        }
    }

    return 0;
}