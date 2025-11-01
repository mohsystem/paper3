#include <stdio.h>
#include <string.h>
#include <unistd.h>  // For open, write, close, ftruncate
#include <fcntl.h>   // For O_RDWR, O_CREAT
#include <sys/stat.h> // For S_IRUSR, S_IWUSR, mkdir, rmdir
#include <errno.h>   // For errno
#include <stdbool.h> // For bool type

/**
 * Opens a file for reading and writing, creating it if it doesn't exist.
 * It then writes the given content to the file.
 *
 * @param file_name The name of the file to open/create.
 * @param content   The string content to write to the file.
 * @return true if the operation was successful, false otherwise.
 */
bool write_config(const char* file_name, const char* content) {
    // O_RDWR: Open for reading and writing.
    // O_CREAT: Create the file if it does not exist.
    // S_IRUSR | S_IWUSR: File permissions 0600 (owner can read/write).
    // This is a security best practice.
    int fd = open(file_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);

    if (fd == -1) {
        // perror prints a descriptive error message to stderr.
        perror("Error opening file");
        return false;
    }

    // Truncate the file to 0 length to ensure we are writing fresh content
    if (ftruncate(fd, 0) == -1) {
        perror("Error truncating file");
        close(fd);
        return false;
    }

    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);

    // Always close the file descriptor, even on error.
    int close_status = close(fd);
    if (close_status == -1) {
        perror("Error closing file");
        // Even if close fails, we report the write status primarily.
    }

    if (bytes_written == -1) {
        perror("Error writing to file");
        return false;
    }

    if ((size_t)bytes_written != content_len) {
        fprintf(stderr, "Incomplete write to file '%s'\n", file_name);
        return false;
    }

    return close_status != -1;
}

int main() {
    const char* content = "important_config";
    const char* test_files[] = {
        "config1.txt",
        "test/config2.txt", // Note: This will fail if 'test' directory doesn't exist.
        "config3.txt",
        "config4.txt",
        "config5.txt"
    };
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);

    // Create a directory for the second test case
    // 0755 permissions: owner-rwx, group-rx, other-rx
    mkdir("test", 0755);

    for (int i = 0; i < num_tests; ++i) {
        printf("Attempting to write to: %s\n", test_files[i]);
        bool result = write_config(test_files[i], content);
        printf("  -> Success: %s\n", result ? "true" : "false");
        if (result) {
            remove(test_files[i]); // Clean up created file
        }
    }

    // Clean up the created directory
    rmdir("test");

    return 0;
}