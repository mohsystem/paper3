#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// POSIX headers for file control
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Creates a file, writes a simple shell script, and makes it executable.
 * This implementation uses POSIX-specific functions (open, write, fchmod, close)
 * to securely create the file and set permissions, mitigating TOCTOU vulnerabilities.
 *
 * @param filename The name of the file to create. Must not contain path separators.
 * @return true on success, false on failure.
 */
bool create_and_make_executable(const char* filename) {
    if (filename == NULL || filename[0] == '\0' || strchr(filename, '/') != NULL || strchr(filename, '\\') != NULL) {
        fprintf(stderr, "Error: Invalid filename. It must not be NULL, empty, or contain path separators.\n");
        return false;
    }
    
    // Clean up file if it exists
    remove(filename);

    // Create the file with read/write permissions for the owner (0600).
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        perror("Error opening file");
        return false;
    }

    char content[256];
    int len = snprintf(content, sizeof(content), "#!/bin/sh\necho \"Hello from %s!\"\n", filename);
    if (len < 0 || (size_t)len >= sizeof(content)) {
        fprintf(stderr, "Error creating content string.\n");
        close(fd);
        return false;
    }

    ssize_t bytes_written = write(fd, content, strlen(content));
    if (bytes_written == -1 || (size_t)bytes_written != strlen(content)) {
        perror("Error writing to file");
        close(fd);
        return false;
    }
    
    // Get current permissions to add execute permission
    struct stat file_stat;
    if (fstat(fd, &file_stat) == -1) {
        perror("Error getting file status");
        close(fd);
        return false;
    }

    // Add execute permission for the owner
    if (fchmod(fd, file_stat.st_mode | S_IXUSR) == -1) {
        perror("Error setting file permissions");
        close(fd);
        return false;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return false;
    }

    return true;
}

int main() {
    const char* test_filenames[] = {
        "script1.sh",
        ".hidden_script.sh",
        "no_extension",
        "",                // Invalid: empty
        "test/script.sh"   // Invalid: contains path separator
    };
    int num_tests = sizeof(test_filenames) / sizeof(test_filenames[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* filename = test_filenames[i];
        printf("Testing with filename: \"%s\"\n", filename);
        bool success = create_and_make_executable(filename);
        printf("Result: %s\n", success ? "SUCCESS" : "FAILURE");
        if (success) {
            printf("File '%s' created and made executable.\n", filename);
            // Clean up the created file
            if (remove(filename) != 0) {
                perror("Error cleaning up file");
            } else {
                printf("Cleaned up '%s'.\n", filename);
            }
        }
        printf("--------------------\n");
    }

    return 0;
}