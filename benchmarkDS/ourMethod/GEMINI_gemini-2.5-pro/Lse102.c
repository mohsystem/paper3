#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

// Define a boolean type for convenience
#if !defined(__cplusplus)
#include <stdbool.h>
#endif

/**
 * Creates a file if it doesn't exist with specific permissions (0644),
 * opens it for reading/writing, and writes content to it.
 * This function is designed to be secure against TOCTOU vulnerabilities
 * by using O_NOFOLLOW and fstat.
 *
 * @param filename The name of the file to open/create.
 * @param content  The string content to write to the file.
 * @return true on success, false on failure.
 */
bool create_and_write_file(const char* filename, const char* content) {
    int fd = -1;
    bool success = false;
    
    // Define flags: Read/Write, Create if not exists, and do not follow symlinks.
    // O_NOFOLLOW is a crucial security feature to prevent symlink attacks.
    int flags = O_RDWR | O_CREAT | O_NOFOLLOW;

    // Define permissions: S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH (0644)
    // Owner: read/write, Group: read, Others: read
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    // Open the file, getting a file descriptor.
    fd = open(filename, flags, mode);
    if (fd == -1) {
        perror("Error opening file");
        goto cleanup;
    }

    // Security check: After opening, validate the file handle.
    // This prevents race conditions where the file could be replaced
    // by a symlink or directory between a check and its use.
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Error getting file stats");
        goto cleanup;
    }
    
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a regular file.\n", filename);
        goto cleanup;
    }

    // Truncate the file to 0 bytes before writing to ensure it only contains new content.
    if (ftruncate(fd, 0) == -1) {
        perror("Error truncating file");
        goto cleanup;
    }
    
    // Write the content to the file.
    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    if (bytes_written == -1) {
        perror("Error writing to file");
        goto cleanup;
    }

    if ((size_t)bytes_written != content_len) {
        fprintf(stderr, "Error: Partial write to '%s'.\n", filename);
        goto cleanup;
    }
    
    success = true;

cleanup:
    if (fd != -1) {
        if (close(fd) == -1) {
            perror("Error closing file");
            // If we succeeded before this, we should now report failure.
            success = false; 
        }
    }
    return success;
}

int main() {
    // Test Case 1: The primary specified case
    printf("Test Case 1: important_config\n");
    if (create_and_write_file("important_config", "important_config")) {
        printf("  Success.\n");
    } else {
        printf("  Failure.\n");
    }

    // Test Case 2: A different filename and content
    printf("\nTest Case 2: another_config.txt\n");
    if (create_and_write_file("another_config.txt", "some other data")) {
        printf("  Success.\n");
    } else {
        printf("  Failure.\n");
    }

    // Test Case 3: Empty content
    printf("\nTest Case 3: empty_file.cfg\n");
    if (create_and_write_file("empty_file.cfg", "")) {
        printf("  Success.\n");
    } else {
        printf("  Failure.\n");
    }

    // Test Case 4: File with special characters in name
    printf("\nTest Case 4: file-with-hyphen.log\n");
    if (create_and_write_file("file-with-hyphen.log", "log entry 1")) {
        printf("  Success.\n");
    } else {
        printf("  Failure.\n");
    }
        
    // Test Case 5: Overwriting an existing file
    printf("\nTest Case 5: Overwriting important_config\n");
    if (create_and_write_file("important_config", "new content")) {
        printf("  Success.\n");
    } else {
        printf("  Failure.\n");
    }

    return 0;
}