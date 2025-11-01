#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

/**
 * Creates a file, writes secret information, and sets permissions to read-only for the owner.
 * @param secret_info The secret string to write.
 * @return An allocated string with the file path on success, NULL on failure.
 *         The caller is responsible for freeing the returned string.
 */
char* create_secret_file(const char* secret_info) {
    // Template for the temporary file name.
    char filename_template[] = "/tmp/secretfile-c.XXXXXX";
    
    // Create a unique temporary file securely with mkstemp.
    // This creates the file with 0600 permissions, avoiding race conditions.
    int fd = mkstemp(filename_template);
    if (fd == -1) {
        perror("Failed to create temporary file");
        return NULL;
    }
    
    // Write the secret info to the file.
    size_t info_len = strlen(secret_info);
    ssize_t bytes_written = write(fd, secret_info, info_len);
    if (bytes_written == -1) {
        perror("Failed to write to temporary file");
        close(fd);
        unlink(filename_template); // Clean up on failure
        return NULL;
    }
    if ((size_t)bytes_written != info_len) {
        fprintf(stderr, "Incomplete write to temporary file.\n");
        close(fd);
        unlink(filename_template); // Clean up on failure
        return NULL;
    }

    // Change permissions to read-only for the owner (0400).
    // fchmod operates on the file descriptor, which is safer than using the path.
    if (fchmod(fd, S_IRUSR) == -1) {
        perror("Failed to change file permissions");
        close(fd);
        unlink(filename_template); // Clean up on failure
        return NULL;
    }
    
    // Close the file descriptor.
    if (close(fd) == -1) {
        perror("Warning: Failed to close file descriptor");
    }

    // Return a dynamically allocated copy of the file path.
    char* file_path = strdup(filename_template);
    if (file_path == NULL) {
        perror("Failed to allocate memory for file path");
        unlink(filename_template); // Cannot return path, so remove the file
        return NULL;
    }
    
    return file_path;
}

int main() {
    printf("Running C test cases...\n");
    char* created_files[5] = {NULL};
    int file_count = 0;

    for (int i = 0; i < 5; ++i) {
        printf("\n--- Test Case %d ---\n", i + 1);
        char secret_buffer[100];
        // Use snprintf for safe string formatting.
        snprintf(secret_buffer, sizeof(secret_buffer), "C super secret info #%d", i + 1);
        
        char* file_name = create_secret_file(secret_buffer);
        if (file_name) {
            printf("Secret file created: %s\n", file_name);
            created_files[file_count++] = file_name; // Store for cleanup
            
            struct stat st;
            if (stat(file_name, &st) == 0) {
                printf("File permissions (octal): %o\n", st.st_mode & 0777);
            } else {
                 perror("Could not stat file");
            }
        } else {
            fprintf(stderr, "Failed to create secret file for test case %d.\n", i + 1);
        }
    }

    // Cleanup
    printf("\nCleaning up created files...\n");
    for (int i = 0; i < file_count; ++i) {
        if (created_files[i]) {
            if (unlink(created_files[i]) == 0) {
                printf("Removed: %s\n", created_files[i]);
            } else {
                fprintf(stderr, "Error removing file %s: %s\n", created_files[i], strerror(errno));
            }
            free(created_files[i]); // Free the memory allocated by strdup
            created_files[i] = NULL;
        }
    }
    printf("\nC tests finished.\n");

    return 0;
}