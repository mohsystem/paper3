#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// For secure file I/O and random number generation on POSIX systems
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

// On Linux, getrandom is available. On other POSIX systems, fall back to /dev/urandom.
#if defined(__linux__) && __has_include(<sys/random.h>)
#include <sys/random.h>
#define USE_GETRANDOM 1
#endif

/**
 * Fills a buffer with cryptographically secure random bytes.
 * @param buf The buffer to fill.
 * @param len The number of bytes to generate.
 * @return true on success, false on failure.
 */
bool get_secure_random_bytes(void* buf, size_t len) {
#if USE_GETRANDOM
    ssize_t bytes_read = getrandom(buf, len, 0);
    if (bytes_read < 0 || (size_t)bytes_read != len) {
        return false;
    }
    return true;
#else
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open /dev/urandom");
        return false;
    }
    ssize_t bytes_read = read(fd, buf, len);
    close(fd);
    if (bytes_read < 0 || (size_t)bytes_read != len) {
        return false;
    }
    return true;
#endif
}

/**
 * Validates a filename to prevent path traversal.
 * @param filename The filename to check.
 * @return true if the filename is simple and does not contain path components.
 */
bool is_valid_filename(const char* filename) {
    if (filename == NULL || filename[0] == '\0') return false;
    // Rule #1: Prevent path traversal.
    if (strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL ||
        strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        return false;
    }
    return true;
}

/**
 * Generates 3 random floats, concatenates them, and writes to a file securely.
 * @param fileName The name of the file to create.
 * @return true if successful, false otherwise.
 */
bool generateAndWriteRandomFloats(const char* fileName) {
    if (!is_valid_filename(fileName)) {
        fprintf(stderr, "Error: Invalid file name '%s'. Path components are not allowed.\n", fileName);
        return false;
    }

    // 1. Generate 3 random floats.
    float random_floats[3];
    for (int i = 0; i < 3; ++i) {
        uint32_t random_int;
        if (!get_secure_random_bytes(&random_int, sizeof(random_int))) {
            fprintf(stderr, "Error: Failed to generate secure random numbers.\n");
            return false;
        }
        random_floats[i] = (float)random_int / (float)UINT32_MAX;
    }

    // 2 & 3. Convert to strings and concatenate into a buffer.
    char buffer[128];
    int offset = 0;
    int remaining = sizeof(buffer);
    for (int i = 0; i < 3; ++i) {
        int written = snprintf(buffer + offset, remaining, "%f", random_floats[i]);
        if (written < 0 || written >= remaining) {
            fprintf(stderr, "Error: snprintf buffer overflow during string construction.\n");
            return false;
        }
        offset += written;
        remaining -= written;
    }

    // 4. Write to file securely.
    // Rules #2, #3: Use open() with O_CREAT | O_EXCL for a race-safe operation.
    int fd = open(fileName, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        fprintf(stderr, "Error opening file %s: %s\n", fileName, strerror(errno));
        return false;
    }
    
    ssize_t content_len = strlen(buffer);
    ssize_t bytes_written = write(fd, buffer, content_len);
    if (bytes_written < 0 || (size_t)bytes_written != content_len) {
        fprintf(stderr, "Error writing to file %s: %s\n", fileName, strerror(errno));
        close(fd);
        unlink(fileName); // Clean up partially written file.
        return false;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "Error closing file %s: %s\n", fileName, strerror(errno));
        return false;
    }

    return true;
}

void run_test_case(const char* fileName) {
    unlink(fileName); // Clean up before test.

    printf("Attempting to write to: %s\n", fileName);
    if (generateAndWriteRandomFloats(fileName)) {
        printf("Successfully wrote to %s\n", fileName);
        FILE* f = fopen(fileName, "r");
        if (f) {
            char buffer[256] = {0};
            if (fgets(buffer, sizeof(buffer), f) != NULL) {
                 printf("File content: %s\n", buffer);
            } else if (feof(f)) {
                 printf("File content: (empty)\n");
            } else {
                 fprintf(stderr, "Failed to read back file content.\n");
            }
            fclose(f);
        } else {
            fprintf(stderr, "Failed to open file for reading back.\n");
        }
    } else {
        fprintf(stderr, "Failed to write to %s\n", fileName);
    }
    printf("---\n");
}


int main(void) {
    printf("Running C test cases...\n");
    
    const char* testFiles[] = {
        "c_test1.txt", "c_test2.txt", "c_test3.txt", "c_test4.txt", "c_test5.txt"
    };
    int num_tests = sizeof(testFiles) / sizeof(testFiles[0]);

    for (int i = 0; i < num_tests; ++i) {
        run_test_case(testFiles[i]);
    }

    // Test case with an invalid path.
    printf("Testing invalid path...\n");
    if (!generateAndWriteRandomFloats("../invalid_test.txt")) {
        printf("Correctly failed to write to invalid path.\n");
    } else {
        fprintf(stderr, "Incorrectly allowed writing to invalid path.\n");
    }

    return 0;
}