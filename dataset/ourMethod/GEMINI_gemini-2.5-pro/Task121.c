#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <libgen.h>

// Define a reasonable file size limit to prevent resource exhaustion.
#define MAX_FILE_SIZE (10 * 1024 * 1024) // 10 MB
// Define a buffer size for file copy operation.
#define BUFFER_SIZE 8192

int uploadFile(const char* source_path, const char* dest_dir_path);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source_file> <destination_directory>\n\n", argv[0]);
        fprintf(stderr, "--- Test Cases ---\n");
        fprintf(stderr, "To run the test cases, first set up the environment:\n");
        fprintf(stderr, "  mkdir -p /tmp/upload_dest\n");
        fprintf(stderr, "  echo \"hello world\" > /tmp/source_file.txt\n");
        fprintf(stderr, "  ln -s /tmp/source_file.txt /tmp/source_symlink\n");
        fprintf(stderr, "  dd if=/dev/zero of=/tmp/large_file.bin bs=1M count=11\n");
        fprintf(stderr, "  mkdir -p /tmp/no_write_dest && chmod 555 /tmp/no_write_dest\n\n");
        
        fprintf(stderr, "1. Test Case: Successful Upload\n");
        fprintf(stderr, "   ./%s /tmp/source_file.txt /tmp/upload_dest\n\n", argv[0]);
        
        fprintf(stderr, "2. Test Case: Source File Does Not Exist\n");
        fprintf(stderr, "   ./%s /tmp/non_existent_file.txt /tmp/upload_dest\n\n", argv[0]);
        
        fprintf(stderr, "3. Test Case: Destination Directory Does Not Exist\n");
        fprintf(stderr, "   ./%s /tmp/source_file.txt /tmp/non_existent_dir\n\n", argv[0]);
        
        fprintf(stderr, "4. Test Case: Source is a Symlink (should be rejected)\n");
        fprintf(stderr, "   ./%s /tmp/source_symlink /tmp/upload_dest\n\n", argv[0]);

        fprintf(stderr, "5. Test Case: File Too Large\n");
        fprintf(stderr, "   ./%s /tmp/large_file.bin /tmp/upload_dest\n\n", argv[0]);

        fprintf(stderr, "After testing, clean up:\n");
        fprintf(stderr, "  rm -f /tmp/source_file.txt /tmp/source_symlink /tmp/large_file.bin\n");
        fprintf(stderr, "  rm -rf /tmp/upload_dest /tmp/no_write_dest\n");

        return EXIT_FAILURE;
    }

    const char* source_file = argv[1];
    const char* dest_dir = argv[2];

    if (uploadFile(source_file, dest_dir) == 0) {
        return EXIT_SUCCESS;
    } else {
        fprintf(stderr, "Upload failed.\n");
        return EXIT_FAILURE;
    }
}

/**
 * @brief Securely uploads a file from a source path to a destination directory.
 *
 * This function follows secure file handling practices to prevent TOCTOU race
 * conditions and path traversal vulnerabilities.
 * 1. It opens the source file with O_NOFOLLOW to reject symbolic links.
 * 2. It validates the opened file descriptor (fstat) to ensure it's a regular file
 *    and within size limits.
 * 3. It creates a temporary file in the destination directory.
 * 4. It copies the data to the temporary file.
 * 5. It uses an atomic rename operation to move the file to its final destination.
 *
 * @param source_path The path to the file to be uploaded.
 * @param dest_dir_path The path to the destination directory.
 * @return 0 on success, -1 on failure.
 */
int uploadFile(const char* source_path, const char* dest_dir_path) {
    int src_fd = -1;
    int dest_dir_fd = -1;
    int temp_fd = -1;
    char *source_path_copy = NULL;
    char *final_basename = NULL;
    char *temp_path_template = NULL;
    int result = -1; // Default to failure

    if (!source_path || !dest_dir_path) {
        fprintf(stderr, "Error: NULL path provided.\n");
        return -1;
    }

    // Use a copy for basename() as it can modify the input string.
    source_path_copy = strdup(source_path);
    if (!source_path_copy) {
        perror("strdup for source path");
        goto cleanup;
    }
    final_basename = basename(source_path_copy);
    if (strcmp(final_basename, ".") == 0 || strcmp(final_basename, "..") == 0 || strlen(final_basename) == 0) {
        fprintf(stderr, "Error: Invalid source filename '%s'.\n", final_basename);
        goto cleanup;
    }

    // 1. Open the destination directory to get a file descriptor as a trust anchor.
    dest_dir_fd = open(dest_dir_path, O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (dest_dir_fd < 0) {
        perror("Error opening destination directory");
        goto cleanup;
    }

    // 2. Open source file securely, rejecting symbolic links.
    src_fd = open(source_path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
    if (src_fd < 0) {
        perror("Error opening source file");
        goto cleanup;
    }

    // 3. Validate the opened source file handle (not the path).
    struct stat src_stat;
    if (fstat(src_fd, &src_stat) < 0) {
        perror("Error getting source file stats");
        goto cleanup;
    }
    if (!S_ISREG(src_stat.st_mode)) {
        fprintf(stderr, "Error: Source is not a regular file.\n");
        goto cleanup;
    }
    if (src_stat.st_size > MAX_FILE_SIZE) {
        fprintf(stderr, "Error: Source file size exceeds the limit of %ld bytes.\n", (long)MAX_FILE_SIZE);
        goto cleanup;
    }

    // 4. Create a temporary file in the destination directory.
    size_t template_len = strlen(dest_dir_path) + 20; // Room for "/upload.XXXXXX"
    temp_path_template = malloc(template_len);
    if (!temp_path_template) {
        perror("malloc for temp path");
        goto cleanup;
    }
    int written_chars = snprintf(temp_path_template, template_len, "%s/upload.XXXXXX", dest_dir_path);
    if (written_chars < 0 || (size_t)written_chars >= template_len) {
         fprintf(stderr, "Error: Failed to construct temp path.\n");
         goto cleanup;
    }

    temp_fd = mkstemp(temp_path_template);
    if (temp_fd < 0) {
        perror("Error creating temporary file");
        goto cleanup;
    }
    
    // Get the basename of the temporary file for renameat().
    const char *temp_basename = strrchr(temp_path_template, '/');
    if (!temp_basename) {
        temp_basename = temp_path_template;
    } else {
        temp_basename++; // Point after the '/'
    }

    // 5. Copy file content from source to temporary file.
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        char *p = buffer;
        ssize_t bytes_to_write = bytes_read;
        while (bytes_to_write > 0) {
            ssize_t bytes_written = write(temp_fd, p, bytes_to_write);
            if (bytes_written < 0) {
                if (errno == EINTR) continue; // Interrupted, try again.
                perror("Error writing to temporary file");
                goto cleanup;
            }
            bytes_to_write -= bytes_written;
            p += bytes_written;
        }
    }
    if (bytes_read < 0) {
        perror("Error reading from source file");
        goto cleanup;
    }

    // 6. Sync data to disk to ensure it's written before rename.
    if (fsync(temp_fd) != 0) {
        perror("Error syncing temporary file");
        goto cleanup;
    }

    // 7. Atomically rename the temporary file to its final name in the destination dir.
    if (renameat(dest_dir_fd, temp_basename, dest_dir_fd, final_basename) != 0) {
        perror("Error renaming temporary file");
        goto cleanup;
    }
    
    // 8. Sync the directory to ensure the rename operation is persisted.
    if (fsync(dest_dir_fd) != 0) {
        perror("Error syncing destination directory");
        goto cleanup;
    }

    printf("File '%s' uploaded successfully to '%s'.\n", final_basename, dest_dir_path);
    result = 0; // Success!

cleanup:
    if (src_fd != -1) close(src_fd);
    if (dest_dir_fd != -1) close(dest_dir_fd);
    if (temp_fd != -1) {
        close(temp_fd);
        // If we failed before the rename, the temp file still exists and must be removed.
        if (result != 0 && temp_path_template) {
            unlink(temp_path_template);
        }
    }
    free(source_path_copy);
    free(temp_path_template);
    return result;
}