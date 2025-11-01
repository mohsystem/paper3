#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// POSIX headers for secure file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

static const char* SAFE_DIR = "/safe/";
static const off_t MAX_FILE_SIZE = 1024 * 1024; // 1 MiB

/**
 * Reads a file from a secure directory, preventing path traversal and other attacks.
 * The caller is responsible for freeing the returned buffer.
 *
 * @param filename The name of the file to read. Must not contain path separators.
 * @param content_size Pointer to a size_t to store the size of the read content.
 * @return A dynamically allocated buffer with the file content, or NULL on failure.
 */
char* readFileFromSafeDirectory(const char* filename, size_t* content_size) {
    int dir_fd = -1;
    int file_fd = -1;
    char* buffer = NULL;
    *content_size = 0;

    // Rule #3: Validate input
    if (filename == NULL || filename[0] == '\0' || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        fprintf(stderr, "Error: Invalid filename.\n");
        return NULL;
    }

    // Get a file descriptor for the safe base directory
    dir_fd = open(SAFE_DIR, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dir_fd < 0) {
        perror("Error opening safe directory");
        goto cleanup;
    }

    // Rule #2: Open-then-validate pattern to avoid TOCTOU
    // O_NOFOLLOW prevents following symbolic links
    file_fd = openat(dir_fd, filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (file_fd < 0) {
        fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
        goto cleanup;
    }

    // Validate the opened handle (file descriptor)
    struct stat st;
    if (fstat(file_fd, &st) != 0) {
        perror("Error getting file status");
        goto cleanup;
    }

    // Rule #1: Ensure it's a regular file
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: Not a regular file.\n");
        goto cleanup;
    }

    // Rule #5: Check file size against a limit
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: File size exceeds limit of %ld bytes.\n", (long)MAX_FILE_SIZE);
        goto cleanup;
    }
    
    if (st.st_size == 0) {
        // Return an empty, but valid, buffer for empty files.
        buffer = malloc(1);
        if (buffer) {
            buffer[0] = '\0';
            *content_size = 0;
        }
        goto cleanup; // Successfully handled empty file
    }

    // Check for potential integer overflow before allocation
    if ((size_t)st.st_size + 1 < (size_t)st.st_size) {
        fprintf(stderr, "Error: File size leads to integer overflow.\n");
        goto cleanup;
    }
    
    buffer = malloc((size_t)st.st_size + 1);
    if (buffer == NULL) {
        perror("Error allocating memory for file content");
        goto cleanup;
    }
    
    ssize_t total_bytes_read = 0;
    while (total_bytes_read < st.st_size) {
        ssize_t bytes_read = read(file_fd, buffer + total_bytes_read, (size_t)st.st_size - total_bytes_read);
        if (bytes_read < 0) {
            if (errno == EINTR) continue; // Interrupted by signal, retry
            perror("Error reading from file");
            free(buffer);
            buffer = NULL;
            goto cleanup;
        }
        if (bytes_read == 0) { // Unexpected EOF
            break;
        }
        total_bytes_read += bytes_read;
    }
    
    buffer[total_bytes_read] = '\0'; // Null-terminate the string
    *content_size = total_bytes_read;

cleanup:
    // Rule #9: Ensure resources are cleaned up
    if (file_fd != -1) close(file_fd);
    if (dir_fd != -1) close(dir_fd);
    return buffer;
}


int main() {
    /*
     * To run these test cases, you need to set up the environment.
     * You may need root permissions to create /safe.
     *
     * sudo mkdir -p /safe/subdir
     * sudo sh -c 'echo "This is a valid file." > /safe/valid_file.txt'
     * sudo sh -c 'echo "This is outside the safe dir." > /outside_file.txt'
     * sudo ln -s /safe/valid_file.txt /safe/symlink_to_valid
     * sudo chmod 600 /safe/valid_file.txt
     */
    const char* test_filenames[] = {
        "valid_file.txt",                // 1. Should succeed
        "non_existent_file.txt",         // 2. Should fail (file not found)
        "../outside_file.txt",           // 3. Should fail (invalid filename)
        "/etc/passwd",                   // 4. Should fail (invalid filename)
        "symlink_to_valid",              // 5. Should fail (is a symlink, O_NOFOLLOW)
        "subdir"                         // 6. Should fail (is a directory)
    };
    int num_tests = sizeof(test_filenames) / sizeof(test_filenames[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* filename = test_filenames[i];
        printf("--- Testing: %s ---\n", filename);
        size_t content_size;
        char* content = readFileFromSafeDirectory(filename, &content_size);

        if (content != NULL) {
            printf("Success! Read %zu bytes.\n", content_size);
            // printf("Content: %s\n", content);
            free(content);
        } else {
            printf("Failed as expected.\n");
        }
    }

    return 0;
}