#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// For mkdir, stat
#include <sys/stat.h>
#include <sys/types.h>

// For open, openat, write, close
#include <fcntl.h>
#include <unistd.h>

// For basename
#include <libgen.h>

// For errno
#include <errno.h>

/**
 * @brief Securely uploads a file to a designated 'uploads' directory.
 * The 'uploads' directory is created in the current working directory.
 * This function prevents path traversal attacks and race conditions (TOCTOU)
 * by using openat() relative to a directory file descriptor.
 *
 * @param filename The name of the file to be saved. This will be sanitized.
 * @param content The byte content of the file.
 * @param content_len The length of the content.
 * @return true if the upload was successful, false otherwise.
 */
bool upload_file(const char* filename, const char* content, size_t content_len) {
    if (filename == NULL || filename[0] == '\0' || content == NULL) {
        fprintf(stderr, "Error: Filename or content is null or empty.\n");
        return false;
    }

    const char* uploads_dir_name = "uploads";
    int dir_fd = -1;
    int file_fd = -1;
    char* filename_copy = NULL;
    bool success = false;

    // Check if 'uploads' exists and is a directory.
    struct stat st = {0};
    if (stat(uploads_dir_name, &st) == -1) {
        // If it doesn't exist, create it with rwx-r-x-r-x permissions.
        if (mkdir(uploads_dir_name, 0755) != 0) {
            perror("Error creating uploads directory");
            return false;
        }
    } else if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "Error: 'uploads' exists but is not a directory.\n");
        return false;
    }

    // Open the uploads directory to get a file descriptor.
    // This is the base for the secure openat() call.
    dir_fd = open(uploads_dir_name, O_RDONLY | O_DIRECTORY);
    if (dir_fd == -1) {
        perror("Error opening uploads directory");
        return false;
    }

    // Sanitize the filename to prevent path traversal.
    // basename() can modify its argument, so we pass a copy.
    filename_copy = strdup(filename);
    if (filename_copy == NULL) {
        perror("Error duplicating filename string");
        close(dir_fd);
        return false;
    }
    char* sanitized_filename = basename(filename_copy);
    if (sanitized_filename == NULL || sanitized_filename[0] == '\0' || strcmp(sanitized_filename, ".") == 0 || strcmp(sanitized_filename, "..") == 0) {
        fprintf(stderr, "Error: Filename is invalid after sanitization.\n");
        goto cleanup;
    }

    // Use openat() to create the file relative to the directory descriptor.
    // This is the most secure way to prevent TOCTOU and path traversal issues.
    // O_CREAT | O_EXCL: Atomically create, fail if exists.
    // O_WRONLY: Write-only.
    // S_IRUSR | S_IWUSR (0600): Permissions for owner read/write.
    file_fd = openat(dir_fd, sanitized_filename, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);
    if (file_fd == -1) {
        perror("Error opening file with openat");
        goto cleanup;
    }

    // Write content to the file.
    ssize_t bytes_written = write(file_fd, content, content_len);
    if (bytes_written < 0 || (size_t)bytes_written != content_len) {
        perror("Error writing to file");
        // Attempt to clean up partially created file.
        unlinkat(dir_fd, sanitized_filename, 0);
        goto cleanup;
    }

    printf("Successfully uploaded file '%s' to '%s' directory.\n", sanitized_filename, uploads_dir_name);
    success = true;

cleanup:
    if (file_fd != -1) close(file_fd);
    if (dir_fd != -1) close(dir_fd);
    if (filename_copy != NULL) free(filename_copy);
    return success;
}

int main() {
    printf("--- Running Test Cases ---\n");
    
    const char* content1 = "This is a valid test file.";
    const char* content2 = "This should fail.";
    const char* content3 = "This should be saved as myfile.txt in uploads";

    // Test Case 1: Valid upload
    printf("\n--- Test Case 1: Valid upload ---\n");
    upload_file("test1.txt", content1, strlen(content1));

    // Test Case 2: Path traversal attempt
    printf("\n--- Test Case 2: Path traversal attempt ---\n");
    upload_file("../test2.txt", content2, strlen(content2));
    upload_file("subdir/../../test2_b.txt", content2, strlen(content2));

    // Test Case 3: Empty/null inputs
    printf("\n--- Test Case 3: Empty/null inputs ---\n");
    upload_file("", content2, strlen(content2));
    upload_file(NULL, content2, strlen(content2));
    upload_file("test3.txt", NULL, 0);


    // Test Case 4: Re-uploading the same file (should fail)
    printf("\n--- Test Case 4: Re-uploading same file ---\n");
    upload_file("test1.txt", content2, strlen(content2));

    // Test Case 5: Filename with path components (should be sanitized)
    printf("\n--- Test Case 5: Filename with path components ---\n");
    upload_file("/etc/passwd", content2, strlen(content2));
    upload_file("C:\\Users\\test\\myfile.txt", content3, strlen(content3));

    printf("\n--- Test Cases Finished ---\n");
    return 0;
}