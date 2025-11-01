#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// POSIX headers for file control
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// For error reporting
#include <errno.h>

/**
 * @brief Opens/creates a file for reading and writing with specific permissions, 
 *        and writes content to it.
 * 
 * @param filename The path to the file.
 * @param content The string content to write.
 * @return true if the operation was successful, false otherwise.
 */
bool createAndWriteFile(const char* filename, const char* content) {
    // Flags: Open for Read/Write, Create if not exists, Truncate if exists.
    int flags = O_RDWR | O_CREAT | O_TRUNC;
    
    // Permissions: rw-r--r-- (0644)
    // Corresponds to S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

    // The third argument (mode) is only used when O_CREAT is specified.
    int fd = open(filename, flags, mode);
    if (fd == -1) {
        fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
        return false;
    }

    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    
    // It's crucial to attempt closing the file descriptor even if write fails.
    int close_status = close(fd);

    if (bytes_written == -1) {
        fprintf(stderr, "Error writing to file '%s': %s\n", filename, strerror(errno));
        return false;
    }

    if ((size_t)bytes_written != content_len) {
        fprintf(stderr, "Error: Partial write to file '%s'\n", filename);
        return false;
    }
    
    if (close_status == -1) {
        fprintf(stderr, "Error closing file '%s': %s\n", filename, strerror(errno));
        return false;
    }

    return true;
}

int main() {
    const char* test_files[][2] = {
        {"important_config", "important_config"},
        {"test_c_1.cfg", "some_data_1"},
        {"test_c_2.cfg", "some_data_2"},
        {"test_c_3.cfg", "some_data_3"},
        {"test_c_4.cfg", "some_data_4"}
    };
    int num_tests = sizeof(test_files) / sizeof(test_files[0]);

    for (int i = 0; i < num_tests; ++i) {
        const char* filename = test_files[i][0];
        const char* content = test_files[i][1];

        printf("--- C Test Case: %s ---\n", filename);
        bool success = createAndWriteFile(filename, content);
        printf("Operation successful: %s\n", success ? "true" : "false");

        if (success) {
            if (remove(filename) != 0) {
                fprintf(stderr, "Failed to clean up file: %s\n", filename);
            } else {
                printf("Cleaned up file: %s\n", filename);
            }
        }
        printf("\n");
    }

    return 0;
}