#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Required for POSIX file operations
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

/**
 * Opens or creates a file for reading and writing, then writes content to it.
 *
 * @param filename The path to the file.
 * @param content The string content to write.
 * @return true on success, false on failure.
 */
bool secureWriteToFile(const char* filename, const char* content) {
    if (filename == NULL || *filename == '\0') {
        fprintf(stderr, "Error: Filename cannot be null or empty.\n");
        return false;
    }
     if (content == NULL) {
        content = ""; // Treat null content as an empty string
    }

    int fd = -1;
    bool success = false;

    // O_RDWR: Open for reading and writing.
    // O_CREAT: Create the file if it does not exist.
    // S_IRUSR | S_IWUSR: Permissions for new file (0600) - owner can read/write.
    fd = open(filename, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error opening file");
        goto cleanup;
    }

    size_t bytes_to_write = strlen(content);
    ssize_t bytes_written = write(fd, content, bytes_to_write);

    if (bytes_written == -1) {
        perror("Error writing to file");
        goto cleanup;
    }

    if ((size_t)bytes_written != bytes_to_write) {
        fprintf(stderr, "Error: Incomplete write to file '%s'.\n", filename);
        goto cleanup;
    }

    success = true;

cleanup:
    if (fd != -1) {
        if (close(fd) == -1) {
            perror("Error closing file");
            // If the write succeeded but close failed, we still mark it as a failure.
            success = false;
        }
    }
    return success;
}

int main() {
    printf("--- C Test Cases ---\n");

    // Test Case 1: Create a new file and write content.
    printf("Test 1: %s\n", secureWriteToFile("config1.txt", "important_config") ? "SUCCESS" : "FAILURE");

    // Test Case 2: Create another file with different content.
    printf("Test 2: %s\n", secureWriteToFile("config2.txt", "user=admin;password=secret") ? "SUCCESS" : "FAILURE");

    // Test Case 3: Overwrite the beginning of the first file.
    printf("Test 3: %s\n", secureWriteToFile("config1.txt", "new_config") ? "SUCCESS" : "FAILURE");

    // Test Case 4: Attempt to write to a file in a non-existent directory (should fail).
    printf("Test 4: %s\n", secureWriteToFile("non_existent_dir/config.txt", "data") ? "SUCCESS" : "FAILURE");

    // Test Case 5: Write an empty string to a file.
    printf("Test 5: %s\n", secureWriteToFile("config5.txt", "") ? "SUCCESS" : "FAILURE");

    return 0;
}