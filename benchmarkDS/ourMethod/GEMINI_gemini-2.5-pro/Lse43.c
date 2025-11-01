#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// POSIX headers for file operations
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

// This code is POSIX-specific (Linux, macOS, etc.)
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#define SAFE_DIRECTORY "/safe/"
#define MAX_FILE_SIZE (1024 * 1024) // 1 MB
#define BUFFER_SIZE 8192

/**
 * @brief Result structure to hold file content and its size.
 */
typedef struct {
    char* content;
    size_t size;
} FileContent;

/**
 * @brief Securely reads a file from a designated safe directory.
 * @param filename The name of the file to read. Must be a relative path without slashes.
 * @return A FileContent struct. On success, content is a dynamically allocated
 *         buffer that the caller must free. On failure, content is NULL.
 */
FileContent read_file_from_safe_directory(const char* filename) {
    FileContent result = {NULL, 0};
    int dir_fd = -1;
    int fd = -1;
    char* buffer = NULL;
    size_t total_bytes_read = 0;
    size_t capacity = 0;

    // Rule #4, #7: Input validation.
    if (filename == NULL || filename[0] == '\0' || strchr(filename, '/') != NULL) {
        fprintf(stderr, "Error: Invalid filename provided.\n");
        return result;
    }

    dir_fd = open(SAFE_DIRECTORY, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dir_fd < 0) {
        perror("Error: Could not open safe directory");
        goto cleanup;
    }

    // Rule #3: Open the resource first.
    fd = openat(dir_fd, filename, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "Error opening file '%s': %s\n", filename, strerror(errno));
        goto cleanup;
    }

    // Rule #3: Validate the opened handle.
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Error getting file status");
        goto cleanup;
    }

    // Rule #4: Reject if not a regular file.
    if (!S_ISREG(st.st_mode)) {
        fprintf(stderr, "Error: '%s' is not a regular file.\n", filename);
        goto cleanup;
    }

    // Rule #1: Check initial size against policy.
    if (st.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: '%s' is too large.\n", filename);
        goto cleanup;
    }
    
    // Rule #1: Read from the validated handle, checking size at each step.
    while (1) {
        if (total_bytes_read == capacity) {
            if (capacity >= MAX_FILE_SIZE) {
                // We've already read up to the limit, so stop.
                break;
            }
            size_t new_capacity = capacity + BUFFER_SIZE;
            if (new_capacity > MAX_FILE_SIZE) {
                new_capacity = MAX_FILE_SIZE;
            }
            char* new_buffer = realloc(buffer, new_capacity);
            if (new_buffer == NULL) {
                perror("Error: Failed to reallocate memory");
                goto cleanup;
            }
            buffer = new_buffer;
            capacity = new_capacity;
        }

        ssize_t bytes_read = read(fd, buffer + total_bytes_read, capacity - total_bytes_read);
        if (bytes_read < 0) {
            if (errno == EINTR) continue;
            perror("Error reading file");
            goto cleanup;
        }
        if (bytes_read == 0) { // EOF
            break;
        }
        total_bytes_read += bytes_read;
    }

    result.content = buffer;
    result.size = total_bytes_read;
    buffer = NULL; // Ownership transferred to result

cleanup:
    if (fd != -1) close(fd);
    if (dir_fd != -1) close(dir_fd);
    free(buffer); // Free if an error occurred before ownership transfer
    return result;
}

void setup_test_environment() {
    // This setup may require appropriate permissions to create directories/files.
    mkdir(SAFE_DIRECTORY, 0755);

    char good_file_path[256];
    snprintf(good_file_path, sizeof(good_file_path), "%sgoodfile.txt", SAFE_DIRECTORY);
    int fd = open(good_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        write(fd, "hello world", 11);
        close(fd);
    }

    char outside_file_path[] = "/tmp/outside.txt";
    fd = open(outside_file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd != -1) {
        write(fd, "secret", 6);
        close(fd);
    }

    char symlink_path[256];
    snprintf(symlink_path, sizeof(symlink_path), "%ssymlink.txt", SAFE_DIRECTORY);
    unlink(symlink_path);
    symlink(outside_file_path, symlink_path);
    
    char subdir_path[256];
    snprintf(subdir_path, sizeof(subdir_path), "%ssubdir", SAFE_DIRECTORY);
    mkdir(subdir_path, 0755);
}

void run_test_case(int num, const char* description, const char* filename) {
    printf("\n%d. %s: '%s'\n", num, description, filename);
    FileContent fc = read_file_from_safe_directory(filename);
    if (fc.content != NULL) {
        printf("Success! Content: \"%.*s\"\n", (int)fc.size, fc.content);
        free(fc.content);
    } else {
        printf("Failed as expected.\n");
    }
}

int main() {
    setup_test_environment();
    printf("--- Running Test Cases ---\n");
    
    run_test_case(1, "Reading 'goodfile.txt' (should succeed)", "goodfile.txt");
    run_test_case(2, "Reading 'nonexistent.txt' (should fail)", "nonexistent.txt");
    run_test_case(3, "Reading 'symlink.txt' (should fail)", "symlink.txt");
    run_test_case(4, "Reading '../goodfile.txt' (should fail)", "../goodfile.txt");
    run_test_case(5, "Reading 'subdir' (should fail)", "subdir");

    return 0;
}