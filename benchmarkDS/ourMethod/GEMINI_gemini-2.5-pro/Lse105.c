#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

// POSIX headers for low-level file I/O
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Creates a file with the given name and content, setting restrictive permissions atomically.
 * This implementation is for POSIX-compliant systems.
 *
 * @param filename The name of the file to create.
 * @param content  The string content to write to the file.
 * @return 0 on success, -1 on failure.
 */
int create_secret_file(const char* filename, const char* content) {
    // Flags for atomic, exclusive creation with write-only access.
    // O_CREAT: Create file if it does not exist.
    // O_EXCL: Error if file exists.
    // O_WRONLY: Open for writing only.
    int flags = O_WRONLY | O_CREAT | O_EXCL;
    
    // Permissions: S_IRUSR | S_IWUSR corresponds to 0600 (read/write for owner only).
    mode_t mode = S_IRUSR | S_IWUSR;

    int fd = open(filename, flags, mode);

    if (fd == -1) {
        fprintf(stderr, "Error: Could not create file '%s': %s\n", filename, strerror(errno));
        return -1;
    }

    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    
    if (bytes_written == -1) {
        fprintf(stderr, "Error: Could not write to file '%s': %s\n", filename, strerror(errno));
        close(fd);
        unlink(filename); // Attempt to clean up the partially written file
        return -1;
    }

    if ((size_t)bytes_written != content_len) {
        fprintf(stderr, "Error: Incomplete write to file '%s'\n", filename);
        close(fd);
        unlink(filename);
        return -1;
    }

    if (close(fd) == -1) {
        fprintf(stderr, "Warning: Could not close file descriptor for '%s': %s\n", filename, strerror(errno));
    }

    printf("Successfully created and secured file: %s\n", filename);
    return 0;
}

int main() {
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    printf("--- Running Test Cases ---\n");

    char test_files[5][20];
    for (int i = 0; i < 5; ++i) {
        snprintf(test_files[i], sizeof(test_files[i]), "secret%d.txt", i + 1);
    }

    // Test Cases 1-5: Create files
    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test: Create %s ---\n", test_files[i]);
        create_secret_file(test_files[i], content);
    }
    
    // Additional Test: Try creating an existing file to test failure
    printf("\n--- Test: Attempt to re-create %s (should fail) ---\n", test_files[1]);
    create_secret_file(test_files[1], content);

    printf("\n--- Cleaning up created files ---\n");
    for (int i = 0; i < 5; ++i) {
        remove(test_files[i]);
    }
    printf("Cleanup complete.\n");

    return 0;
}