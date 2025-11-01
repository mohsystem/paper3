#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// POSIX headers for low-level file I/O
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

// Header for error handling
#include <errno.h>

/**
 * Creates/opens a file with specific permissions (0644) and writes content.
 * The file is opened for reading and writing, created if it doesn't exist,
 * and truncated if it does.
 * 
 * @param filename The name of the file.
 * @param content The content to write to the file.
 * @return true on success, false on failure.
 */
bool create_and_write_config(const char* filename, const char* content) {
    // Flags: Read/Write, Create if not exists, Truncate if exists.
    int flags = O_RDWR | O_CREAT | O_TRUNC;
    
    // Mode: Owner rw, Group r, Others r (0644 in octal)
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
    
    // The third argument (mode) is only used when O_CREAT is specified.
    int fd = open(filename, flags, mode);
    
    if (fd == -1) {
        perror("Error opening file");
        return false;
    }
    
    size_t content_len = strlen(content);
    ssize_t bytes_written = write(fd, content, content_len);
    
    // Always close the file descriptor, even if write fails
    int close_status = close(fd);
    
    if (bytes_written == -1) {
        perror("Error writing to file");
        return false;
    }
    
    if ((size_t)bytes_written != content_len) {
        fprintf(stderr, "Error: Incomplete write to %s\n", filename);
        return false;
    }

    if (close_status == -1) {
        perror("Error closing file");
        return false; // Though write succeeded, close failed, which is a problem
    }
    
    return true;
}

void run_test_case(int test_num) {
    const char* filename = "important_config";
    const char* content = "important_config";

    printf("\n--- Test Case %d ---\n", test_num);
    printf("Attempting to write '%s' to '%s'\n", content, filename);
    
    if (create_and_write_config(filename, content)) {
        printf("Successfully wrote to file.\n");
        // Optional verification using standard C I/O
        FILE* fp = fopen(filename, "r");
        if (fp) {
            char buffer[128]; // Buffer large enough for content
            if (fgets(buffer, sizeof(buffer), fp)) {
                // Remove potential newline if present
                buffer[strcspn(buffer, "\n")] = 0;
                printf("File content verified: \"%s\"\n", buffer);
            } else {
                 fprintf(stderr, "Failed to read content for verification.\n");
            }
            fclose(fp);
        } else {
            perror("Failed to open file for verification");
        }
    } else {
        fprintf(stderr, "Failed to write to file.\n");
    }
}

int main() {
    printf("Running 5 test cases...\n");
    for (int i = 1; i <= 5; ++i) {
        run_test_case(i);
    }
    return 0;
}