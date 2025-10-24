#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdint.h>

#define MAX_FILE_SIZE 1048576 // 1 MiB

/**
 * @brief Validates a filename against a strict whitelist.
 *
 * It allows only alphanumeric characters, dots, underscores, and hyphens.
 * It rejects empty/NULL names, names that are too long, names that are "." or "..",
 * and any names containing path separators ('/' or '\'). This helps prevent
 * path traversal attacks.
 *
 * @param filename The filename to validate.
 * @return 1 if the filename is valid, 0 otherwise.
 */
int isValidFilename(const char *filename) {
    if (!filename || filename[0] == '\0') {
        return 0; // NULL or empty string
    }

    size_t len = strlen(filename);
    if (len > NAME_MAX) { // NAME_MAX from <limits.h>
        return 0; // Filename too long
    }

    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        return 0; // Reject "." and ".."
    }

    for (size_t i = 0; i < len; ++i) {
        char c = filename[i];
        if (!(isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-')) {
            // Disallow path separators and other special characters
            return 0;
        }
    }

    return 1;
}

/**
 * @brief Reads the entire content of a file into a dynamically allocated buffer.
 *
 * This function takes a filename, validates it, and securely opens and reads the file.
 * It follows the "open-then-validate" pattern to avoid TOCTOU race conditions.
 * It checks that the target is a regular file, is not a symbolic link, and does not exceed
 * a maximum size limit.
 *
 * @param filename The name of the file to read. Must be in the current directory.
 * @return A dynamically allocated string containing the file content, or NULL on error.
 *         The caller is responsible for freeing the returned buffer.
 */
char* readFileContent(const char* filename) {
    int fd = -1;
    char *buffer = NULL;
    struct stat st;

    if (!isValidFilename(filename)) {
        fprintf(stderr, "Error: Invalid filename provided.\n");
        return NULL;
    }

    // Rule #6: Open first, then validate handle. O_NOFOLLOW prevents symlink traversal.
    fd = open(filename, O_RDONLY | O_CLOEXEC | O_NOFOLLOW);
    if (fd < 0) {
        perror("Error opening file");
        return NULL;
    }

    // Get file metadata from the file descriptor
    if (fstat(fd, &st) != 0) {
        perror("Error getting file stats");
        goto cleanup;
    }

    // Rule #6: Validate handle - ensure it's a regular file
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Path does not point to a regular file.\n");
        goto cleanup;
    }

    // Rule #3: Validate size to prevent excessive memory usage
    if (st.st_size < 0) {
        fprintf(stderr, "Error: Invalid file size (negative).\n");
        goto cleanup;
    }
    if ((uintmax_t)st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File is too large (max %d bytes).\n", MAX_FILE_SIZE);
        goto cleanup;
    }

    size_t content_size = (size_t)st.st_size;
    size_t buffer_size = content_size + 1;

    // Handle empty file case
    if (content_size == 0) {
        buffer = malloc(1);
        if (buffer) {
            buffer[0] = '\0';
        } else {
            perror("Error allocating memory for empty file");
        }
        close(fd);
        return buffer;
    }

    // Rule #3: Allocate buffer, checking for allocation failure
    buffer = malloc(buffer_size);
    if (!buffer) {
        perror("Error allocating memory for file content");
        goto cleanup;
    }

    // Rule #3: Read file content within buffer boundaries
    ssize_t bytes_read = read(fd, buffer, content_size);
    if (bytes_read < 0) {
        perror("Error reading from file");
        goto cleanup;
    }

    if ((size_t)bytes_read != content_size) {
        fprintf(stderr, "Error: Incomplete read from file.\n");
        goto cleanup;
    }

    buffer[bytes_read] = '\0'; // Null-terminate the string

    close(fd);
    return buffer;

cleanup:
    // Centralized cleanup for error paths
    if (buffer) free(buffer);
    if (fd >= 0) close(fd);
    return NULL;
}

// --- Test Suite ---
void setup_test_files() {
    int fd;
    // Test case 1: A valid file
    fd = open("test_valid.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        const char *content = "This is a valid file.";
        write(fd, content, strlen(content));
        close(fd);
    }
    // Test case 2: An empty file
    fd = open("test_empty.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        close(fd);
    }
    // Test case 3: A large file
    fd = open("test_large.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        char buf[1024] = {0};
        for (int i = 0; i <= (MAX_FILE_SIZE / 1024); ++i) {
            write(fd, buf, sizeof(buf));
        }
        close(fd);
    }
    // Test case 4: A directory
    mkdir("test_dir", 0755);
    // Test case 5: A symbolic link
    symlink("test_valid.txt", "test_symlink.txt");
}

void cleanup_test_files() {
    unlink("test_valid.txt");
    unlink("test_empty.txt");
    unlink("test_large.bin");
    unlink("test_symlink.txt");
    rmdir("test_dir");
}

void run_tests() {
    printf("--- Running Integrated Tests ---\n");
    setup_test_files();

    const char* test_cases[] = {
        "test_valid.txt",          // 1. Should succeed
        "test_empty.txt",          // 2. Should succeed with empty content
        "non_existent.txt",        // 3. Should fail (file not found)
        "test_large.bin",          // 4. Should fail (file too large)
        "test_dir",                // 5. Should fail (not a regular file)
        "test_symlink.txt",        // 6. Should fail (symlink rejected)
        "../invalid_path.txt"      // 7. Should fail (invalid filename format)
    };
    int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    int passed = 0;

    for (int i = 0; i < num_tests; ++i) {
        printf("\n--- Test Case %d: Reading '%s' ---\n", i + 1, test_cases[i]);
        char* content = readFileContent(test_cases[i]);
        int expected_to_pass = (i < 2); // First two cases should pass

        if (content) {
            if (expected_to_pass) {
                printf("SUCCESS (as expected).\n");
                passed++;
            } else {
                printf("FAILURE (unexpectedly passed).\n");
            }
            printf("Content length: %zu\n", strlen(content));
            free(content);
        } else {
            if (!expected_to_pass) {
                printf("FAILURE (as expected).\n");
                passed++;
            } else {
                printf("FAILURE (unexpectedly failed).\n");
            }
        }
    }
    
    printf("\n--- Test Summary: %d/%d passed ---\n", passed, num_tests);

    cleanup_test_files();
}

int main(int argc, char *argv[]) {
    if (argc == 2 && strcmp(argv[1], "--test") == 0) {
        run_tests();
        return 0;
    }

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        fprintf(stderr, "Or run tests with: %s --test\n", argv[0]);
        return 1;
    }

    char *filename = argv[1];
    printf("Attempting to read file: %s\n", filename);

    char *file_data = readFileContent(filename);

    if (file_data) {
        printf("--- File Content ---\n%s\n--- End of Content ---\n", file_data);
        free(file_data);
        return 0;
    } else {
        fprintf(stderr, "Operation failed.\n");
        return 1;
    }
}