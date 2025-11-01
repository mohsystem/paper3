#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// POSIX headers for file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * @brief Creates a file if it doesn't exist, opens it for reading/writing,
 * and writes content to it.
 *
 * This function follows secure coding practices:
 * - It uses open() with restrictive permissions (0600).
 * - It validates the file descriptor immediately after opening using fstat()
 *   to ensure it's a regular file, preventing TOCTOU vulnerabilities.
 * - It checks the return values of all system calls.
 *
 * @param path The path to the file.
 * @param content The string content to write.
 * @return 0 on success, -1 on failure.
 */
int create_and_write_file(const char *path, const char *content) {
    if (path == NULL || content == NULL) {
        fprintf(stderr, "Error: path or content is NULL.\n");
        return -1;
    }

    // Open the file with flags: Read/Write, Create if not exists.
    // Set restrictive permissions (S_IRUSR | S_IWUSR corresponds to 0600).
    // O_NOFOLLOW prevents opening a symbolic link, a security best practice.
    int fd = open(path, O_RDWR | O_CREAT | O_NOFOLLOW, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    // After opening, immediately validate the handle (file descriptor).
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error getting file stats");
        close(fd);
        // Attempt to remove the file that was just created but is in an error state.
        remove(path);
        return -1;
    }

    // Ensure we are operating on a regular file.
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: %s is not a regular file.\n", path);
        close(fd);
        // Clean up the created file system entry.
        remove(path);
        return -1;
    }

    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);

    if (bytes_written == -1) {
        perror("Error writing to file");
        close(fd);
        return -1;
    }

    if ((size_t)bytes_written != content_len) {
        fprintf(stderr, "Error: Incomplete write to file %s\n", path);
        close(fd);
        return -1;
    }

    // Close the file descriptor.
    if (close(fd) == -1) {
        perror("Error closing file");
        return -1;
    }

    return 0;
}

int main() {
    const char *important_config = "important_config=true";

    // Test cases
    const char *test_files[] = {
        "test_config1.txt",
        "test_config2.txt",
        "test_config3.txt",
        "test_config4.txt",
        "test_config5.txt"
    };
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);

    for (int i = 0; i < num_tests; ++i) {
        printf("Attempting to write to: %s\n", test_files[i]);
        if (create_and_write_file(test_files[i], important_config) == 0) {
            printf("Successfully wrote to %s\n", test_files[i]);
            // Clean up the test file
            if (remove(test_files[i]) != 0) {
                perror("Error cleaning up file");
            } else {
                printf("Cleaned up %s\n", test_files[i]);
            }
        } else {
            fprintf(stderr, "Failed to write to %s\n", test_files[i]);
        }
        printf("--------------------\n");
    }

    return 0;
}