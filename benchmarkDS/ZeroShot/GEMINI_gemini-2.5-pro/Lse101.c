#include <stdio.h>
#include <string.h>
#include <errno.h>

// This code is designed for POSIX-compliant systems (like Linux, macOS)
// to ensure atomic file creation with secure permissions.
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

/**
 * Creates a file and writes content to it with secure permissions atomically.
 * This function is designed for POSIX-compliant systems.
 * Permissions are set to owner-only read/write (0600 or S_IRUSR | S_IWUSR).
 *
 * @param fileName The name of the file to write to.
 * @param content  The null-terminated string content to write to the file.
 * @return 0 on success, -1 on failure.
 */
int writeSecureFile(const char* fileName, const char* content) {
    if (fileName == NULL || *fileName == '\0' || content == NULL) {
        fprintf(stderr, "Error: File name or content cannot be null or empty.\n");
        return -1;
    }

    int fd = -1;
    // open() is atomic: it creates the file and sets permissions in one step.
    // O_WRONLY: Open for writing only.
    // O_CREAT: Create the file if it does not exist.
    // O_TRUNC: If the file exists, truncate it to 0 bytes.
    // S_IRUSR | S_IWUSR: Read/write permissions for the owner only (0600).
    fd = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    size_t contentLen = strlen(content);
    ssize_t bytes_written = write(fd, content, contentLen);

    if (bytes_written == -1) {
        perror("Error writing to file");
        close(fd); // Attempt to close even on error
        return -1;
    }

    if ((size_t)bytes_written != contentLen) {
        fprintf(stderr, "Error: Incomplete write to file '%s'.\n", fileName);
        close(fd);
        return -1;
    }

    if (close(fd) == -1) {
        perror("Error closing file");
        return -1;
    }

    return 0;
}

void runTestCase(const char* fileName, const char* content) {
    printf("Running test for file: '%s'\n", (fileName != NULL ? fileName : "null"));
    int result = writeSecureFile(fileName, content);
    if (result == 0) {
        printf(" -> Success.\n");
    } else {
        printf(" -> Failed.\n");
    }
    printf("\n");
}

int main() {
    printf("--- C Test Cases ---\n");
    
    runTestCase("important_config", "important_config");
    runTestCase("user_settings.conf", "user=admin;level=5;");
    runTestCase("api_key.txt", "ABC-123-DEF-456");
    runTestCase("log_file.log", "Initial log entry.");
    runTestCase(NULL, "some content"); // Invalid filename

    return 0;
}