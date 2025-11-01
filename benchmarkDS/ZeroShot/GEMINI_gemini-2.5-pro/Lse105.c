#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Platform-specific includes and definitions for file operations
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

/**
 * Creates a file, writes content, and sets restrictive permissions securely.
 * This implementation uses low-level file operations to set permissions at
 * creation time, avoiding TOCTOU race conditions.
 *
 * @param filename The name of the file to create.
 * @param content  The string content to write to the file.
 * @return true if successful, false otherwise.
 */
bool createAndSecureFile(const char* filename, const char* content) {
    int fd;
    
#ifdef _WIN32
    // On Windows, _open creates a file with specified permissions.
    // _S_IREAD | _S_IWRITE corresponds to read/write for the owner.
    fd = _open(filename, _O_WRONLY | _O_CREAT | _O_TRUNC, _S_IREAD | _S_IWRITE);
#else
    // On POSIX, open does the same. S_IRUSR | S_IWUSR is mode 0600.
    fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
#endif

    if (fd == -1) {
        perror("Error opening file");
        return false;
    }

    size_t content_len = strlen(content);
    ssize_t bytes_written;

#ifdef _WIN32
    bytes_written = _write(fd, content, (unsigned int)content_len);
#else
    bytes_written = write(fd, content, content_len);
#endif
    
    int close_status;
#ifdef _WIN32
    close_status = _close(fd);
#else
    close_status = close(fd);
#endif

    if (bytes_written == -1) {
        perror("Error writing to file");
        return false;
    }
    if ((size_t)bytes_written != content_len) {
        fprintf(stderr, "Error: Incomplete write to file %s\n", filename);
        return false;
    }
    if (close_status == -1) {
        perror("Error closing file");
        return false;
    }

    printf("Successfully created and secured file %s\n", filename);
    return true;
}

int main() {
    const char* content = "U2FsdGVkX1/R+WzJcxgvX/Iw==";

    // 5 Test Cases: Create 5 separate files to demonstrate the function.
    for (int i = 1; i <= 5; ++i) {
        char filename[20];
        snprintf(filename, sizeof(filename), "secret%d.txt", i);

        printf("\n--- Test Case %d: Creating %s ---\n", i, filename);
        bool success = createAndSecureFile(filename, content);
        if (success) {
            printf("Test Case %d PASSED.\n", i);
        } else {
            printf("Test Case %d FAILED.\n", i);
        }
    }

    return 0;
}